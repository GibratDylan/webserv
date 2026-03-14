<?php
session_start();

// If already authenticated, go straight to the protected page.
if (isset($_SESSION['auth']) && $_SESSION['auth'] === true) {
    header('Location: /profile.php');
    exit;
}

$error = '';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $username = isset($_POST['username']) ? trim($_POST['username']) : '';
    $password = isset($_POST['password']) ? $_POST['password'] : '';

    // Demo credentials for local testing.
    $validUser = 'admin';
    $validPass = 'qwerty42';

    if ($username === $validUser && $password === $validPass) {
        $_SESSION['auth'] = true;
        $_SESSION['username'] = $username;
        $_SESSION['login_time'] = time();

        header('Location: /profile.php');
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
            font-family: Arial, sans-serif;
            max-width: 540px;
            margin: 60px auto;
            padding: 0 16px;
        }
        form {
            border: 1px solid #dcdcdc;
            border-radius: 8px;
            padding: 16px;
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
    <h1>Login</h1>
    <p>Demo credentials: <code>admin / qwerty42</code></p>

    <form method="POST" action="/login.php">
        <label for="username">Username</label>
        <input id="username" name="username" type="text" required>

        <label for="password">Password</label>
        <input id="password" name="password" type="password" required>

        <button type="submit">Sign in</button>

        <?php if ($error !== ''): ?>
            <p class="error"><?php echo htmlspecialchars($error, ENT_QUOTES, 'UTF-8'); ?></p>
        <?php endif; ?>
    </form>
</body>
</html>
