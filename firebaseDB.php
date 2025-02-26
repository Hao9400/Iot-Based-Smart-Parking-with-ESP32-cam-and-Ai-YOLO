<?php
require __DIR__ . '/vendor/autoload.php';
use Kreait\Firebase\Factory;

$factory = (new Factory())
    ->withProjectId('fpyhao') // Your Firebase project ID
    ->withDatabaseUri('https://fyphao-default-rtdb.asia-southeast1.firebasedatabase.app') // Correct Firebase URL
    ->withServiceAccount(__DIR__ . '/fyphao-firebase-adminsdk-3gtyi-61bb3d1a6c.json'); // Correct JSON file path

$database = $factory->createDatabase();
?>
