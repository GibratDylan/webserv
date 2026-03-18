#!/usr/bin/env python3

import os

print("Content-Type: text/plain; charset=utf-8")
print()
print("virtual-location-cgi-ok")
print(f"SCRIPT_NAME={os.environ.get('SCRIPT_NAME','')}")
print(f"QUERY_STRING={os.environ.get('QUERY_STRING','')}")
