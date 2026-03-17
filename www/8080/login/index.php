<?php
$sessionId = isset($_COOKIE['session_id']) ? $_COOKIE['session_id'] : '';
$sessionId = preg_replace('/[^a-zA-Z0-9]/', '', $sessionId);

$storageDir = dirname(__DIR__) . '/uploads/session_demo';
if (!is_dir($storageDir)) {
    mkdir($storageDir, 0700, true);
}

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

if (isset($data['auth']) && $data['auth'] === true) {
    header('Location: /profile');
    exit;
}

$error = '';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    if (!$hasSessionId) {
        $error = 'No session_id yet. Reload page once and try again.';
    }

    $username = isset($_POST['username']) ? trim($_POST['username']) : '';
    $password = isset($_POST['password']) ? $_POST['password'] : '';

    $validUser = 'admin';
    $validPass = '42';

    if ($error === '' && $username === $validUser && $password === $validPass) {
        $data['auth'] = true;
        $data['username'] = $username;
        $data['login_time'] = time();
        $data['last_visit'] = date('c');

        file_put_contents($sessionFile, json_encode($data, JSON_PRETTY_PRINT));

        header('Location: /profile');
        exit;
    }

    $error = 'Invalid login or password';
}
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Session Login</title>
    <style>
        body {
            font-family: verdana;
            display: flex;
            align-items:center;
            justify-content: center;
            height: 100vh;
        }
        form {
            border: 1px solid #dcdcdc;
            border-radius: 8px;
            padding: 16px;
            background: #dadaff;
        }
        label {
            display: block;
            margin-top: 12px;
        }
        input {
            width: 100%;
            box-sizing: border-box;
            padding: 8px;
            margin-top: 4px;
        }
        button {
            margin-top: 14px;
            padding: 8px 14px;
            cursor: pointer;

        }
        .error {
            color: #b00020;
            margin-top: 10px;
        }
        code {
            background: #f3f3f3;
            padding: 2px 4px;
            border-radius: 4px;
        }
    </style>
</head>
<body>
    <div>
    <h1>Login</h1>
    <form method="POST" action="">
        <label for="username">Username</label>
        <input id="username" name="username" type="text" required>

        <label for="password">Password</label>
        <input id="password" name="password" type="password" required>

        <button type="submit">Sign in</button>

        <?php if ($error !== ''): ?>
            <p class="error"><?php echo htmlspecialchars($error, ENT_QUOTES, 'UTF-8'); ?></p>
        <?php endif; ?>
    </form>
    <p>Demo credentials: <code>admin / 42</code></p>
        </div>
</body>
</html>
