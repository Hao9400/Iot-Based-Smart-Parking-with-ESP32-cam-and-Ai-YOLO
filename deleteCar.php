<?php
require("firebaseDB.php");
session_start();

// Check if the user is logged in as an admin
if (!isset($_SESSION['Admin-name'])) {
    header("location: login.php");
    exit();
}

// Check if the ID parameter is set in the URL
if (isset($_GET['id'])) {
    $carId = $_GET['id'];

    try {
        // Reference to the specific car record in Firebase
        $reference = $database->getReference('FYPHao/' . $carId);

        // Remove the car record from Firebase
        $reference->remove();

        // Redirect back to the car information page with a success message
        $_SESSION['message'] = "Car record deleted successfully!";
        header("location: userslog.php");
        exit();
    } catch (Exception $e) {
        // Handle any errors that occur during the deletion process
        $_SESSION['error'] = "Error deleting car record: " . $e->getMessage();
        header("location: userslog.php");
        exit();
    }
} else {
    // If no ID is provided, redirect back to the car information page
    $_SESSION['error'] = "No car ID provided for deletion.";
    header("location: userslog.php");
    exit();
}
?>