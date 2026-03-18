#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import os


def main() -> None:
    base_dir = os.path.dirname(os.path.abspath(__file__))
    upload_dir = os.path.normpath(os.path.join(base_dir, "..", "uploads"))
    files = []
    try:
        for name in sorted(os.listdir(upload_dir)):
            if name.startswith('.'):
                continue
            path = os.path.join(upload_dir, name)
            if os.path.isfile(path):
                try:
                    size = os.path.getsize(path)
                except Exception:
                    size = 0
                files.append({"name": name, "size": size})
    except Exception:
        files = []

    print("Content-Type: application/json; charset=utf-8")
    print("Cache-Control: no-store")
    print()
    print(json.dumps({"files": files}, ensure_ascii=False))


if __name__ == "__main__":
    main()
