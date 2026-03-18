#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import re
import sys
import json
import time
from http.cookies import SimpleCookie
from urllib.parse import quote


SESSIONS_DIR = "/tmp/sessions"


def _session_path(session_id: str) -> str:
    return os.path.join(SESSIONS_DIR, f"{session_id}.json")


def load_session(session_id: str) -> dict:
    try:
        with open(_session_path(session_id), "r", encoding="utf-8") as f:
            return json.load(f)
    except Exception:
        return {}


def session_username():
    cookie = SimpleCookie(os.environ.get("HTTP_COOKIE", ""))
    sid = cookie.get("session_id")
    if not sid or not sid.value:
        return None
    return load_session(sid.value).get("username")


def redirect(location: str) -> None:
    print("Status: 302 Found")
    print(f"Location: {location}")
    print("Content-Type: text/plain; charset=utf-8")
    print()
    print("Redirecting...")


def error(status: str, message: str) -> None:
    print(f"Status: {status}")
    print("Content-Type: text/plain; charset=utf-8")
    print()
    print(message)


def safe_filename(name: str) -> str:
    base = os.path.basename(name or "")
    base = base.strip().replace("\x00", "")
    if not base:
        return "upload.bin"
    # Keep it simple and predictable
    base = re.sub(r"[^A-Za-z0-9._-]", "_", base)
    if base in {".", ".."}:
        return "upload.bin"
    return base


def unique_path(folder: str, filename: str) -> str:
    path = os.path.join(folder, filename)
    if not os.path.exists(path):
        return path
    stem, ext = os.path.splitext(filename)
    for i in range(1, 10_000):
        cand = os.path.join(folder, f"{stem}_{i}{ext}")
        if not os.path.exists(cand):
            return cand
    return os.path.join(folder, f"{stem}_{int(time.time())}{ext}")


def main() -> None:
    if os.environ.get("REQUEST_METHOD", "GET").upper() != "POST":
        redirect("/upload/")
        return

    if not session_username():
        redirect("/login/")
        return

    try:
        import cgi  # deprecated, but widely available; simplest for multipart
    except Exception:
        error("500 Internal Server Error", "cgi module not available")
        return

    try:
        form = cgi.FieldStorage(fp=sys.stdin.buffer, environ=os.environ, keep_blank_values=True)
    except Exception as e:
        error("500 Internal Server Error", "Failed to parse multipart: " + repr(e))
        return
    if "file" not in form:
        error("400 Bad Request", "Missing 'file' field")
        return
    field = form["file"]
    if not getattr(field, "filename", None):
        error("400 Bad Request", "No file selected")
        return

    filename = safe_filename(field.filename)

    base_dir = os.path.dirname(os.path.abspath(__file__))
    upload_dir = os.path.normpath(os.path.join(base_dir, "..", "uploads"))
    os.makedirs(upload_dir, exist_ok=True)

    out_path = unique_path(upload_dir, filename)
    try:
        with open(out_path, "wb") as f:
            while True:
                chunk = field.file.read(1024 * 1024)
                if not chunk:
                    break
                f.write(chunk)
    except Exception as e:
        error("500 Internal Server Error", f"Failed to save: {e}")
        return

    saved_name = os.path.basename(out_path)
    redirect("/upload/?file=" + quote(saved_name))


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        # Last-resort error handler so the client gets a readable response.
        print("Status: 500 Internal Server Error")
        print("Content-Type: text/plain; charset=utf-8")
        print()
        print("Unhandled exception: " + repr(e))
        try:
            sys.stderr.write("upload.py unhandled: " + repr(e) + "\n")
        except Exception:
            pass
