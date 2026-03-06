#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import cgi
import cgitb
from datetime import datetime

# Enable CGI traceback for debugging
cgitb.enable()

def main():
    # Print HTTP headers
    print("Content-Type: text/html; charset=utf-8")
    print()  # Empty line to separate headers from body
    
    # Get environment variables
    method = os.environ.get('REQUEST_METHOD', 'GET')
    query_string = os.environ.get('QUERY_STRING', '')
    content_length = os.environ.get('CONTENT_LENGTH', '0')
    
    # Parse form data
    form = cgi.FieldStorage()
    
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
    
    # Display form data if present
    if form:
        print('<h2>📝 Données du formulaire reçues</h2>')
        print('<table>')
        print('<tr><th>Paramètre</th><th>Valeur</th></tr>')
        for key in form.keys():
            value = form.getvalue(key)
            print(f'<tr><td>{cgi.escape(key)}</td><td>{cgi.escape(str(value))}</td></tr>')
        print('</table>')
    
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
