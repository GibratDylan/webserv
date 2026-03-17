<?php
$sessionId = isset($_COOKIE['session_id']) ? $_COOKIE['session_id'] : '';
$sessionId = preg_replace('/[^a-zA-Z0-9]/', '', $sessionId);

$storageDir = dirname(__DIR__) . '/uploads/session_demo';
$hasSessionId = ($sessionId !== '');
$sessionFile = $hasSessionId ? ($storageDir . '/' . $sessionId . '.json') : '';

if ($hasSessionId && file_exists($sessionFile)) {
    $raw = file_get_contents($sessionFile);
    $decoded = json_decode($raw, true);
    $data = is_array($decoded) ? $decoded : array();

    unset($data['auth']);
    unset($data['username']);
    unset($data['login_time']);
    $data['last_visit'] = date('c');

    file_put_contents($sessionFile, json_encode($data, JSON_PRETTY_PRINT));
}

header('Location: /login');
exit;
