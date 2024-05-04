<?php
session_start();

// Generate CSRF token and store it in session
if (!isset($_SESSION['csrf_token'])) {
    $_SESSION['csrf_token'] = bin2hex(random_bytes(32));
}

// Validate CSRF token
function validate_csrf_token($token)
{
    return isset($_SESSION['csrf_token']) && hash_equals($_SESSION['csrf_token'], $token);
}

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $csrf_token = $_POST['csrf_token'] ?? '';
    if (!validate_csrf_token($csrf_token)) {
        die("CSRF token validation failed!");
    }
}

// Define categories
$cats = [
    'electronics' => 'Electronics',
    'apparel' => 'Clothing',
    'entertainment' => 'Entertainment',
    'family' => 'Family',
    'free' => 'Free',
    'garden' => 'Home & Garden',
    'home' => 'Home Goods',
    'home-improvements' => 'Home Improvement',
    'hobbies' => 'Hobbies',
    'instruments' => 'Instrument',
    'propertyforsale' => 'Property',
    'sports' => 'Sporting Goods',
    'toys' => 'Toys',
    'vehicles' => 'Vehicles',
];


$searchTerm = '';
$category = '';

// Check if form is submitted
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Get search term
    $searchTerm = isset($_POST['search']) ? $_POST['search'] : '';

    // Get category
    $category = isset($_POST['custom_cat']) && !empty($_POST['custom_cat']) ? $_POST['custom_cat'] : (isset($_POST['category']) ? $_POST['category'] : '');

    // Build query parameters
    $queryParams = [];
    $queryParams[] = 'deliveryMethod=local_pick_up';
    $queryParams[] = 'query=' . urlencode($searchTerm);
    $queryParams[] = 'sortBy=distance_ascend';
    if (isset($_POST['exact']) && $_POST['exact'] === 'on') {
        $queryParams[] = 'exact=true';
    }

    // Build query string
    $queryString = implode('&', $queryParams);

    // Build URL
    $url = 'https://www.facebook.com/marketplace/';
    if (!empty($category)) {
        $url .= 'category/' . $category . '/';
    }
    $url .= '?' . $queryString;

    // Redirect to constructed URL
    header("Location: $url");
    exit();
}
?>

<!doctype html>
<html lang="en">

<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Facebook Search Sucks</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-QWTKZyjpPEjISv5WaRU9OFeRpok6YctnYmDr5pNlyT2bRjXh0JMhjY6hW+ALEwIH" crossorigin="anonymous">
</head>

<body>


    <div class="row mt-3">
        <div class="col-12 col-md-8 col-lg-4 offset-md-2 offset-lg-4">
            <div class="card">
                <div class="card-header">
                    <h3>Search Facebook Marketplace</h3>
                </div>
                <div class="card-body">
                    <form method="post">
                        <input type="hidden" name="csrf_token" value="<?php echo htmlspecialchars($_SESSION['csrf_token']); ?>">
                        <div class="mb-3">
                            <label for="search" class="form-label">Search Facebook</label>
                            <input type="text" class="form-control" id="search" name="search" value="<?= $searchTerm ?>" autofocus required>
                        </div>
                        <hr>
                        <h6><b>Note:</b> You REALLY want a category whether from the list or one you type in yourself.</h6>
                        <div class="mb-3">
                            <label for="category" class="form-label">Category</label>
                            <select class="form-select" id="category" name="category">
                                <option value="">--None--</option>
                                <?php foreach ($cats as $key => $cat) : ?>
                                    <option value="<?= $key ?>" <?= $category === $key ? 'selected' : '' ?>><?= $cat ?></option>
                                <?php endforeach; ?>
                            </select>
                        </div>

                        <div class="mb-3">
                            <label for="custom_cat" class="form-label">Custom Category</label>
                            <input type="text" class="form-control" id="custom_cat" name="custom_cat" value="<?= isset($_POST['custom_cat']) ? $_POST['custom_cat'] : '' ?>">
                        </div>

                        <div class="mb-3">
                            <input type="checkbox" class="form-check-input" id="exact" name="exact" <?= isset($_POST['exact']) && $_POST['exact'] === 'on' ? 'checked' : '' ?> <?php
                                                                                                                                                                                if (empty($_POST)) {
                                                                                                                                                                                    echo 'checked';
                                                                                                                                                                                }
                                                                                                                                                                                ?>>
                            <label for="exact" class="form-label">Prefer Exact Search Term</label>
                        </div>
                        <button type="submit" class="btn btn-primary">Search</button>
                    </form>
                </div>
                <p class="text-center pt-3">
                    Want to host your own version of this? Check out the <a href="https://github.com/mudmin/AnotherMaker/tree/master/yt_sucks">Get the Code</a>.
                </p>
                <p class="text-center pt-2">
                    If you appreciate my work, feel free to donate at <a href="https://userspice.com/donate/">https://userspice.com/donate/</a>.
                </p>
            </div>
        </div>
    </div>

    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js" integrity="sha384-YvpcrYf0tY3lHB60NNkmXc5s9fDVZLESaAA55NDzOxhy9GkcIdslK1eN7N6jIeHz" crossorigin="anonymous"></script>
</body>

</html>