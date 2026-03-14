<?php
session_start();

if (isset($_SESSION['auth']) && $_SESSION['auth'] === true) {
    header('Location: /profile');
    exit;
}

$error = '';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $username = isset($_POST['username']) ? trim($_POST['username']) : '';
    $password = isset($_POST['password']) ? $_POST['password'] : '';

    $validUser = 'admin';
    $validPass = '42';

    if ($username === $validUser && $password === $validPass) {
        $_SESSION['auth'] = true;
        $_SESSION['username'] = $username;
        $_SESSION['login_time'] = time();

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
