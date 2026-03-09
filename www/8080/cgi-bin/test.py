#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import html
from datetime import datetime
from urllib.parse import parse_qs, unquote

def parse_cookies():
    cookie_header = os.environ.get('HTTP_COOKIE', '')
    cookies = {}
    if cookie_header:
        for cookie in cookie_header.split(';'):
            cookie = cookie.strip()
            if '=' in cookie:
                key, value = cookie.split('=', 1)
                cookies[key.strip()] = value.strip()
    return cookies

def parse_form_data():
    """Parse form data from GET or POST request"""
    method = os.environ.get('REQUEST_METHOD', 'GET')
    form_data = {}
    
    if method == 'GET':
        # Parse query string
        query_string = os.environ.get('QUERY_STRING', '')
        if query_string:
            parsed = parse_qs(query_string)
            # Convert lists to single values
            form_data = {k: v[0] if len(v) == 1 else v for k, v in parsed.items()}
    
    elif method == 'POST':
        try:
            # Read POST data from stdin (use buffer for binary reading)
            content_length = int(os.environ.get('CONTENT_LENGTH', 0))
            if content_length > 0:
                # Read binary data and decode to string
                post_data = sys.stdin.buffer.read(content_length).decode('utf-8')
                content_type = os.environ.get('CONTENT_TYPE', '')
                
                # Store raw data for debugging
                form_data['_raw_post_data'] = post_data
                
                # Try to parse as form data (be more flexible with content type)
                if 'application/x-www-form-urlencoded' in content_type or '=' in post_data:
                    # Even if content-type is wrong, try to parse if it looks like form data
                    parsed = parse_qs(post_data, keep_blank_values=True)
                    form_data.update({k: v[0] if len(v) == 1 else v for k, v in parsed.items()})
        except Exception as e:
            # Log error for debugging
            form_data = {'error': str(e)}
    
    return form_data

def main():
    cookies = parse_cookies()
    raw_visits = cookies.get('visits', '0')
    try:
        visits = int(raw_visits)
    except ValueError:
        visits = 0
    visits += 1

    # Print HTTP headers
    print("Content-Type: text/html; charset=utf-8")
    print(f"Set-Cookie: visits={visits}; Path=/; Max-Age=31536000; SameSite=Lax")
    print()  # Empty line to separate headers from body

    # Get environment variables
    method = os.environ.get('REQUEST_METHOD', 'GET')
    query_string = os.environ.get('QUERY_STRING', '')
    content_length = os.environ.get('CONTENT_LENGTH', '0')

    session_id = cookies.get('session_id', '')
    
    # Parse form data
    form = parse_form_data()
    
    # HTML output
    print("""<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CGI Python Test</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 800px;
            margin: 50px auto;
            padding: 20px;
            background-color: #f0f0f0;
        }
        .container {
            background-color: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            border-bottom: 2px solid #4CAF50;
            padding-bottom: 10px;
        }
        h2 {
            color: #666;
            margin-top: 30px;
        }
        .info {
            background-color: #e8f5e9;
            padding: 15px;
            border-radius: 5px;
            margin: 10px 0;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 10px 0;
        }
        th, td {
            text-align: left;
            padding: 8px;
            border-bottom: 1px solid #ddd;
        }
        th {
            background-color: #4CAF50;
            color: white;
        }
        tr:hover {
            background-color: #f5f5f5;
        }
        .form-section {
            margin-top: 20px;
            padding: 20px;
            background-color: #f9f9f9;
            border-radius: 5px;
        }
        input[type="text"], textarea {
            width: 100%;
            padding: 8px;
            margin: 5px 0;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        input[type="submit"] {
            background-color: #4CAF50;
            color: white;
            padding: 10px 20px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            margin-top: 10px;
        }
        input[type="submit"]:hover {
            background-color: #45a049;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🐍 CGI Python3 Test</h1>
""")
    
    print(f'<div class="info"><strong>Date/Heure:</strong> {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}</div>')
    print(f'<div class="info"><strong>Méthode HTTP:</strong> {method}</div>')
    
    # Display query string if present
    if query_string:
        print(f'<div class="info"><strong>Query String:</strong> {query_string}</div>')
    
    # Display content length for POST requests
    if method == 'POST':
        print(f'<div class="info"><strong>Content-Length:</strong> {content_length}</div>')
        content_type = os.environ.get('CONTENT_TYPE', 'non défini')
        print(f'<div class="info"><strong>Content-Type:</strong> {content_type}</div>')
    
    # Display form data if present
    if form:
        print('<h2>📝 Données du formulaire reçues</h2>')
        
        # Show raw POST data for debugging
        if '_raw_post_data' in form:
            raw_data = form.pop('_raw_post_data')
            print(f'<div class="info" style="background-color: #e3f2fd;"><strong>Données brutes POST:</strong> <code>{html.escape(raw_data)}</code></div>')
        
        if form:
            print('<table>')
            print('<tr><th>Paramètre</th><th>Valeur</th></tr>')
            for key, value in form.items():
                print(f'<tr><td>{html.escape(key)}</td><td>{html.escape(str(value))}</td></tr>')
            print('</table>')
        else:
            print('<div class="info" style="background-color: #fff3cd;">Données POST reçues mais non parsées</div>')
    elif method == 'POST':
        print('<h2>📝 Données du formulaire reçues</h2>')
        print('<div class="info" style="background-color: #fff3cd;">Aucune donnée POST reçue</div>')
    
    # Display environment variables
    print('<h2>🔧 Variables d\'environnement CGI</h2>')
    print('<table>')
    print('<tr><th>Variable</th><th>Valeur</th></tr>')
    
    cgi_vars = [
        'SERVER_SOFTWARE', 'SERVER_NAME', 'GATEWAY_INTERFACE',
        'SERVER_PROTOCOL', 'SERVER_PORT', 'REQUEST_METHOD',
        'PATH_INFO', 'PATH_TRANSLATED', 'SCRIPT_NAME',
        'QUERY_STRING', 'REMOTE_HOST', 'REMOTE_ADDR',
        'AUTH_TYPE', 'REMOTE_USER', 'CONTENT_TYPE',
        'CONTENT_LENGTH', 'HTTP_USER_AGENT', 'HTTP_ACCEPT',
        'HTTP_ACCEPT_LANGUAGE', 'HTTP_HOST'
    ]
    
    for var in cgi_vars:
        value = os.environ.get(var, '<i>non défini</i>')
        print(f'<tr><td><strong>{var}</strong></td><td>{value}</td></tr>')
    
    print('</table>')

    
    print('<details style="margin-top: 40px"><summary><H2 style="display: inline;">Toutes les variables d\'environnement</H2></summary>')
    print('<table>')
    print('<tr><th>Variable</th><th>Valeur</th></tr>')
    for var in sorted(os.environ.keys()):
        value = os.environ.get(var, '')
        print(f'<tr><td><strong>{html.escape(var)}</strong></td><td>{html.escape(value)}</td></tr>')
    print('</table></details>')

    print('<details style="margin-top: 40px"><summary><H2 style="display: inline; ">Cookies </H2></summary>')
    print('<table>')
    print('<tr><th>Variable</th><th>Valeur</th></tr>')
    for var in sorted(cookies.keys()):
        value = cookies.get(var, '')
        print(f'<tr><td><strong>{html.escape(var)}</strong></td><td>{html.escape(value)}</td></tr>')
    print('</table></details>')

    if session_id:
        print(f'<div class="info"><strong> Session ID:</strong> <code>{session_id}</code></div>')
    else:
        print('<div class="info"><strong> Session:</strong> Pas de session (nouvelle visite)</div>')

    print(f'<div class="info"><strong>Visites de cette page (cookie):</strong> {visits}</div>')

    # Test form
    print("""
        <div class="form-section">
            <h2>📬 Formulaire de test</h2>
            <form method="POST" action="/cgi-bin/test.py">
                <label for="name">Nom:</label>
                <input type="text" id="name" name="name" placeholder="Entrez votre nom">
                
                <label for="email">Email:</label>
                <input type="text" id="email" name="email" placeholder="Entrez votre email">
                
                <label for="message">Message:</label>
                <textarea id="message" name="message" rows="4" placeholder="Entrez votre message"></textarea>
                
                <input type="submit" value="Envoyer">
            </form>
        </div>
    """)
    
    # Test with GET
    print("""
        <div class="form-section">
            <h2>🔗 Test GET</h2>
            <form method="GET" action="/cgi-bin/test.py">
                <label for="param1">Paramètre 1:</label>
                <input type="text" id="param1" name="param1" placeholder="Valeur 1">
                
                <label for="param2">Paramètre 2:</label>
                <input type="text" id="param2" name="param2" placeholder="Valeur 2">
                
                <input type="submit" value="Envoyer (GET)">
            </form>
        </div>
    """)
    
    print("""
    </div>
</body>
</html>
""")

if __name__ == '__main__':
    main()
