<?php
require("firebaseDB.php");
session_start();

if (!isset($_SESSION['Admin-name'])) {
    header("location: login.php");
    exit();
}
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <title>Delete Car Information</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" type="text/css" href="css/userslog.css"/>
    <script src="https://code.jquery.com/jquery-3.3.1.js"
            integrity="sha256-2Kok7MbOyxpgUVvAk/HJ2jigOSYS2auK4Pfzbm7uH60="
            crossorigin="anonymous"></script>
    <script type="text/javascript" src="js/bootbox.min.js"></script>
    <script type="text/javascript" src="js/bootstrap.js"></script>
    <style>
        label, input {
            color: black;
        }
        button {
            font-family: "Roboto", sans-serif;
            text-transform: uppercase;
            outline: 0;
            background: #f44336; /* Red color for Delete */
            width: 100%;
            border: 0;
            padding: 15px;
            color: #FFFFFF;
            font-size: 15px;
            -webkit-transition: all 0.3 ease;
            transition: all 0.3 ease;
            cursor: pointer;
            border-radius: 0 25px;
        }
        button:hover {
            background: #d32f2f; /* Darker red on hover */
        }
    </style>
</head>
<body>
<section class="container py-lg-5">
    <h1>Car Information - Delete Record</h1>
    <div>
        <table border="1" class="table">
            <thead>
                <tr>
                    <th>Number Plate</th>
                    <th>Time In</th>
                    <th>Time Out</th>
                    <th>Action</th>
                </tr>
            </thead>
            <tbody>
            <?php
            include 'header.php';
            try {
                $reference = $database->getReference('FYPHao');
                $cars = $reference->getValue();

                if ($cars && is_array($cars)) {
                    foreach ($cars as $key => $car) {
                        echo "<tr>";
                        echo "<form method='POST' action='deleteCar.php?id=".$key."'>"; // Action points to deleteCar.php for deletion
                        if (is_array($car) && isset($car['number_plate'])) {
                            $numberPlate = htmlspecialchars($car['number_plate']);
                        } else {
                            $numberPlate = htmlspecialchars($car);
                        }
                        // Display car data in a non-editable field
                        echo "<td><span>$numberPlate</span></td>";

                        // Display Time In and Time Out, assuming these are stored in the database as 'time_in' and 'time_out'
                        $timeIn = isset($car['time_in']) ? htmlspecialchars($car['time_in']) : 'N/A';
                        $timeOut = isset($car['time_out']) ? htmlspecialchars($car['time_out']) : 'N/A';

                        echo "<td><span>$timeIn</span></td>";
                        echo "<td><span>$timeOut</span></td>";

                        // The delete button
                        echo "<td><button type='submit'>Delete</button></td>";
                        echo "</form>";
                        echo "</tr>";
                    }
                } else {
                    echo '<tr><td colspan="4">No data available</td></tr>';
                }
            } catch (Exception $e) {
                echo '<tr><td colspan="4">Error fetching data: ' . $e->getMessage() . '</td></tr>';
            }
            ?>
            </tbody>
        </table>
    </div>
</section>
</body>
</html>
