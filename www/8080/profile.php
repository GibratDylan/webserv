<?php
session_start();

if (!isset($_SESSION['auth']) || $_SESSION['auth'] !== true) {
    header('Location: /login.php');
    exit;
}

$username = isset($_SESSION['username']) ? $_SESSION['username'] : 'unknown';
$loginTime = isset($_SESSION['login_time']) ? date('Y-m-d H:i:s', (int)$_SESSION['login_time']) : 'unknown';
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
        <p>You are authorized via PHP session.</p>
        <p>Username: <strong><?php echo htmlspecialchars($username, ENT_QUOTES, 'UTF-8'); ?></strong></p>
        <p>Login time: <code><?php echo htmlspecialchars($loginTime, ENT_QUOTES, 'UTF-8'); ?></code></p>
        <a href="/logout.php">Log out</a>
    </div>
</body>
</html>
