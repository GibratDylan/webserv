#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import os
import sys
import time
from http.cookies import SimpleCookie
from urllib.parse import parse_qs


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


def get_server_session_id():
    cookie = SimpleCookie(os.environ.get("HTTP_COOKIE", ""))
    sid = cookie.get("session_id")
    if sid and sid.value:
        return sid.value
    return None


def read_form_urlencoded() -> dict:
    length = int(os.environ.get("CONTENT_LENGTH", "0") or "0")
    if length <= 0:
        return {}
    raw = sys.stdin.buffer.read(length).decode("utf-8", errors="replace")
    parsed = parse_qs(raw, keep_blank_values=True)
    return {k: (v[0] if len(v) == 1 else v) for k, v in parsed.items()}


def redirect(location, extra_headers=None):
    print("Status: 302 Found")
    print(f"Location: {location}")
    if extra_headers:
        for h in extra_headers:
            print(h)
    print("Content-Type: text/plain; charset=utf-8")
    print()
    print("Redirecting...")


def main() -> None:
    if os.environ.get("REQUEST_METHOD", "GET").upper() != "POST":
        redirect("/login/")
        return

    session_id = get_server_session_id()
    if not session_id:
        # Webserv sets session_id on the first request.
        # If the client POSTs without having it yet, ask to retry via /login/.
        redirect("/login/")
        return

    form = read_form_urlencoded()
    username = (form.get("username") or "").strip()
    password = (form.get("password") or "").strip()
    next_url = (form.get("next") or "/profile/").strip() or "/profile/"

    allowed = {
        "demo": "demo",
        "admin": "webserv",
    }
    ok = bool(username) and allowed.get(username) == password

    session = load_session(session_id)

    if ok:
        session["username"] = username
        save_session(session_id, session)
        redirect(next_url)
    else:
        redirect("/login/")


if __name__ == "__main__":
    main()
