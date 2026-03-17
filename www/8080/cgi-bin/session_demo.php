<?php
$sessionId = isset($_COOKIE['session_id']) ? $_COOKIE['session_id'] : '';

$storageDir = dirname(__DIR__) . '/uploads/session_demo';
if (!is_dir($storageDir)) {
    mkdir($storageDir, 0700, true);
}

$hasSessionId = ($sessionId !== '');
$sessionFile = $hasSessionId ? ($storageDir . '/' . $sessionId . '.json') : '';
$data = array(
    'visits' => 0,
    'created_at' => date('c'),
    'last_visit' => date('c')
);

if ($hasSessionId && file_exists($sessionFile)) {
    $raw = file_get_contents($sessionFile);
    $decoded = json_decode($raw, true);
    if (is_array($decoded)) {
        $data = array_merge($data, $decoded);
    }
}

$reset = isset($_GET['reset']) && $_GET['reset'] === '1';
if ($hasSessionId && $reset) {
    if (file_exists($sessionFile)) {
        unlink($sessionFile);
    }
    $data = array(
        'visits' => 0,
        'created_at' => date('c'),
        'last_visit' => date('c')
    );
}

if ($hasSessionId) {
    $data['visits'] = (int)$data['visits'] + 1;
    $data['last_visit'] = date('c');
    file_put_contents($sessionFile, json_encode($data, JSON_PRETTY_PRINT));
}

header('Content-Type: text/html; charset=UTF-8');
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Webserv Session Demo (PHP)</title>
    <style>
        body {
            font-family: Verdana, sans-serif;
            max-width: 760px;
            margin: 40px auto;
            padding: 0 16px;
            line-height: 1.45;
        }
        .box {
            border: 1px solid #d7d7d7;
            border-radius: 8px;
            padding: 14px;
            margin-top: 12px;
            background: #fafafa;
        }
        code {
            background: #f0f0f0;
            padding: 2px 4px;
            border-radius: 4px;
        }
        .hint {
            color: #6a6a6a;
        }
        a {
            margin-right: 12px;
        }
    </style>
</head>
<body>
    <h1>Webserv session_id demo (PHP CGI)</h1>

    <div class="box">
        <p><strong>Cookie from webserv:</strong>
            <?php echo $hasSessionId ? htmlspecialchars($sessionId, ENT_QUOTES, 'UTF-8') : '<em>not available yet</em>'; ?>
        </p>

        <?php if ($hasSessionId): ?>
            <p><strong>Visits in this session:</strong> <?php echo (int)$data['visits']; ?></p>
            <p><strong>Created at:</strong> <code><?php echo htmlspecialchars($data['created_at'], ENT_QUOTES, 'UTF-8'); ?></code></p>
            <p><strong>Last visit:</strong> <code><?php echo htmlspecialchars($data['last_visit'], ENT_QUOTES, 'UTF-8'); ?></code></p>
            <p><strong>Storage file:</strong> <code><?php echo htmlspecialchars($sessionFile, ENT_QUOTES, 'UTF-8'); ?></code></p>
        <?php else: ?>
            <p class="hint">
                On the first request webserv creates <code>session_id</code> in response headers.
                Reload once to send this cookie back and bind the session data.
            </p>
        <?php endif; ?>
    </div>

    <p>
        <a href="/cgi-bin/session_demo.php">Reload</a>
        <a href="/cgi-bin/session_demo.php?reset=1">Reset this session data</a>
    </p>
</body>
</html>
