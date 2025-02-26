<?php
require("firebaseDB.php");
require("databaseUrL.php");
session_start();
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <title>Available Spaces</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" type="text/css" href="css/userslog.css">
    <script type="text/javascript" src="js/jquery-2.2.3.min.js"></script>
    <script src="https://code.jquery.com/jquery-3.3.1.js"></script>
    <script type="text/javascript" src="js/bootstrap.js"></script>
    <!-- Include Chart.js -->
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        table {
            width: 100%;
            text-align: center;
            border-collapse: collapse;
        }
        th, td {
            padding: 15px;
            border: 1px solid black;
        }
        th {
            background-color: #4CAF50;
            color: white;
        }
        .available-box {
            text-align: center;
            font-size: 20px;
            font-weight: bold;
            margin-bottom: 10px;
        }
        .chart-container {
            width: 50%;
            margin: 20px auto;
        }
    </style>
</head>
<body>
<?php include 'header.php'; ?>

<section class="container py-lg-5">
    <h1 class="slideInDown animated">Available Spaces from Firebase</h1>

    <?php
    try {
        // Fetch data from Firebase
        $reference = $database->getReference('available');
        $spaces = $reference->getValue();

        // Extract data safely
        $availableSpaces = isset($spaces['availableSpaces']) ? $spaces['availableSpaces'] : 0;
        $zones = isset($spaces['zones']) ? $spaces['zones'] : [];

        // Extract individual zone values
        $zoneA = isset($zones[0]) ? $zones[0] : 0;
        $zoneB = isset($zones[1]) ? $zones[1] : 0;
        $zoneC = isset($zones[2]) ? $zones[2] : 0;

        // Calculate total spaces and occupied spaces
        $totalSpaces = $zoneA + $zoneB + $zoneC;
        $occupiedSpaces = $totalSpaces - $availableSpaces;
    } catch (Exception $e) {
        echo "<p>Error fetching data: " . $e->getMessage() . "</p>";
        $availableSpaces = 0;
        $zoneA = $zoneB = $zoneC = 0;
        $totalSpaces = 0;
        $occupiedSpaces = 0;
    }
    ?>

    <div class="available-box">
        <p><strong>Available Spaces:</strong> <?php echo $availableSpaces; ?></p>
    </div>

    <div class="slideInRight animated">
        <table border="1">
            <thead>
                <tr>
                    <th>Zone A</th>
                    <th>Zone B</th>
                    <th>Zone C</th>
                </tr>
            </thead>
            <tbody>
                <tr>
                    <td><?php echo $zoneA; ?></td>
                    <td><?php echo $zoneB; ?></td>
                    <td><?php echo $zoneC; ?></td>
                </tr>
            </tbody>
        </table>
    </div>

    <!-- Pie Chart Container -->
    <div class="chart-container">
        <canvas id="zonesPieChart"></canvas>
    </div>
</section>

<script>
    // Data for the pie chart
    const zoneA = <?php echo $zoneA; ?>;
const zoneB = <?php echo $zoneB; ?>;
const zoneC = <?php echo $zoneC; ?>;

// Pie Chart Configuration for Zones
const ctx = document.getElementById('zonesPieChart').getContext('2d');
const zonesPieChart = new Chart(ctx, {
    type: 'pie',
    data: {
        labels: ['Zone A', 'Zone B', 'Zone C'],
        datasets: [{
            data: [zoneA, zoneB, zoneC],
            backgroundColor: [
                '#FF6384', // Red for Zone A
                '#36A2EB', // Blue for Zone B
                '#FFCE56'  // Yellow for Zone C
            ],
            borderColor: [
                '#FFFFFF', // White border
                '#FFFFFF',
                '#FFFFFF'
            ],
            borderWidth: 2
        }]
    },
    options: {
        responsive: true,
        plugins: {
            legend: {
                position: 'top',
                labels: {
                    color: 'black' // Set legend text to black
                }
            },
            title: {
                display: true,
                text: 'Parking Zones Distribution',
                color: 'black' // Set title text to black
            }
        }
    }
});

</script>
</body>
</html>