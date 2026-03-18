#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import os
import time
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


def save_session(session_id: str, data: dict) -> None:
    os.makedirs(SESSIONS_DIR, exist_ok=True)
    tmp = _session_path(session_id) + ".tmp"
    with open(tmp, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, ensure_ascii=False)
    os.replace(tmp, _session_path(session_id))


def get_session_id():
    cookie = SimpleCookie(os.environ.get("HTTP_COOKIE", ""))
    sid = cookie.get("session_id")
    if sid and sid.value:
        return sid.value
    return None


def main() -> None:
    sid = get_session_id()
    if sid:
        session = load_session(sid)
        session.pop("username", None)
        session["logout_at"] = int(time.time())
        save_session(sid, session)

    print("Status: 302 Found")
    print("Location: /")
    print("Content-Type: text/plain; charset=utf-8")
    print()
    print("Logged out")


if __name__ == "__main__":
    main()
