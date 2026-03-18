#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import os
from http.cookies import SimpleCookie


SESSIONS_DIR = "/tmp/sessions"


def _session_path(session_id: str) -> str:
    return os.path.join(SESSIONS_DIR, f"{session_id}.json")


def load_session(session_id: str) -> dict:
    try:
        with open(_session_path(session_id), "r", encoding="utf-8") as f:
            return json.load(f)
    except Exception:
        return {}


def main() -> None:
    cookie = SimpleCookie(os.environ.get("HTTP_COOKIE", ""))
    sid = cookie.get("session_id")
    session = load_session(sid.value) if (sid and sid.value) else {}

    username = session.get("username")
    payload = {
        "loggedIn": bool(username),
        "username": username or "",
    }

    print("Content-Type: application/json; charset=utf-8")
    print("Cache-Control: no-store")
    print()
    print(json.dumps(payload, ensure_ascii=False))


if __name__ == "__main__":
    main()
