<?php
$sessionId = isset($_COOKIE['session_id']) ? $_COOKIE['session_id'] : '';
$sessionId = preg_replace('/[^a-zA-Z0-9]/', '', $sessionId);

$storageDir = dirname(__DIR__) . '/uploads/session_demo';
$hasSessionId = ($sessionId !== '');
$sessionFile = $hasSessionId ? ($storageDir . '/' . $sessionId . '.json') : '';
$data = array();

if ($hasSessionId && file_exists($sessionFile)) {
    $raw = file_get_contents($sessionFile);
    $decoded = json_decode($raw, true);
    if (is_array($decoded)) {
        $data = $decoded;
    }
}

if (!isset($data['auth']) || $data['auth'] !== true) {
    header('Location: /login');
    exit;
}

$username = isset($data['username']) ? $data['username'] : 'unknown';
$loginTime = isset($data['login_time']) ? date('Y-m-d H:i:s', (int)$data['login_time']) : 'unknown';
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Protected Profile</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 700px;
            margin: 60px auto;
            padding: 0 16px;
        }
        .box {
            border: 1px solid #dcdcdc;
            border-radius: 8px;
            padding: 16px;
        }
        code {
            background: #f3f3f3;
            padding: 2px 4px;
            border-radius: 4px;
        }
        a {
            display: inline-block;
            margin-top: 14px;
        }
    </style>
</head>
<body>
    <h1>Protected page</h1>

    <div class="box">
        <p>You are authorized via webserv <code>session_id</code>.</p>
        <p>Username: <strong><?php echo htmlspecialchars($username, ENT_QUOTES, 'UTF-8'); ?></strong></p>
        <p>Login time: <code><?php echo htmlspecialchars($loginTime, ENT_QUOTES, 'UTF-8'); ?></code></p>
        <p>Session file: <code><?php echo htmlspecialchars($sessionFile, ENT_QUOTES, 'UTF-8'); ?></code></p>
        <a href="/profile/logout.php">Log out</a>
    </div>
</body>
</html>
