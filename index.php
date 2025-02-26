<?php
session_start();
if (!isset($_SESSION['Admin-name'])) {
    header("location: login.php");
    exit();
}

require("firebaseDB.php");
require("databaseUrL.php");

// Get the search term if provided
$searchTerm = isset($_POST['search_number_plate']) ? $_POST['search_number_plate'] : '';
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <title>Users</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" type="image/png" href="images/favicon.png">
    <link rel="stylesheet" type="text/css" href="css/Users.css">
    <script type="text/javascript" src="js/jquery-2.2.3.min.js"></script>
    <script type="text/javascript" src="js/bootstrap.js"></script>
    <script>
      $(window).on("load resize", function() {
        var scrollWidth = $('.tbl-content').width() - $('.tbl-content table').width();
        $('.tbl-header').css({'padding-right': scrollWidth});
      }).resize();
    </script>
</head>
<body>
<?php include 'header.php'; ?> 
<main>
<section>
  <h1 class="slideInDown animated">Here are all the Users</h1>
  
  <!-- Add search form -->
  <form method="POST" class="mb-3">
    <input type="text" name="search_number_plate" value="<?php echo htmlspecialchars($searchTerm); ?>" placeholder="Search by number plate..." class="form-control" />
    <button type="submit" class="btn btn-primary mt-2">Search</button>
  </form>

  <!-- User table -->
  <div class="table-responsive slideInRight animated" style="max-height: 400px;"> 
    <table class="table">
      <thead class="table-primary">
        <tr>
          <th>Number Plate</th>
          <th>Time In</th>
          <th>Time Out</th>
        </tr>
      </thead>
      <tbody class="table-secondary">
        <?php
        try {
            // Fetch data from Firebase
            $reference = $database->getReference('FYPHao');  // Replace 'FYPHao' with your Firebase path
            $cars = $reference->getValue();

            // Check if there is a search term and filter the results
            if ($cars && is_array($cars)) {
                foreach ($cars as $car) {
                    // If search term is provided, check if it matches the number_plate
                    if ($searchTerm && (!isset($car['number_plate']) || strpos(strtolower($car['number_plate']), strtolower($searchTerm)) === false)) {
                        continue; // Skip this car if the number plate doesn't match
                    }
                    
                    // Display car details in table rows
                    echo "<tr>";
                    echo "<td>" . (isset($car['number_plate']) ? htmlspecialchars($car['number_plate']) : 'N/A') . "</td>";
                    echo "<td>" . (isset($car['time_in']) ? htmlspecialchars($car['time_in']) : 'N/A') . "</td>";
                    echo "<td>" . (isset($car['time_out']) ? htmlspecialchars($car['time_out']) : 'N/A') . "</td>";
                    echo "</tr>";
                }
            } else {
                echo '<tr><td colspan="3" class="text-center">No records found.</td></tr>';
            }
        } catch (Exception $e) {
            echo '<tr><td colspan="3" class="text-center">Error fetching data: ' . $e->getMessage() . '</td></tr>';
        }
        ?>
      </tbody>
    </table>
  </div>
</section>
</main>
</body>
</html>
