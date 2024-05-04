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

    // Get the URL input
    $url = $_POST["video"];

    // Check if the URL starts with "https://www.youtube" or "https://youtube"
    if (strpos($url, 'https://www.youtube') !== 0 && strpos($url, 'https://youtube') !== 0) {
        // Redirect to the same page with an error message
        header("Location: " . $_SERVER['PHP_SELF'] . "?error=invalid_url");
        exit();
    }

    // Fetch the content of the page
    $content = file_get_contents($url);
    // search for the "canonicalBaseUrl" pattern
    preg_match('/"canonicalBaseUrl":"\/channel\/(UC[\w-]+)"/', $content, $matches);

    // If a match is not found, search for the YouTube channel ID pattern
    if (empty($matches)) {
        preg_match('/youtube\.com\/channel\/(UC[\w-]+)/', $content, $matches);
    }

    // If a match is found, modify the channel ID and redirect to the original URL with the modified list parameter
    if (!empty($matches)) {
        $channelId = $matches[1];
        // Modify the channel ID by replacing the second character with 'U'
        $modifiedChannelId = substr_replace($channelId, 'U', 1, 1);
        // Redirect to the original URL with the modified list parameter
        header("Location: $url&list=$modifiedChannelId");
        exit();
    } else {
        // If the channel ID is not found, redirect to the same page with an error message
        header("Location: " . $_SERVER['PHP_SELF'] . "?error=channel_id_not_found");
        exit();
    }
}

// If there's an error, display the appropriate message
$error = isset($_GET['error']) ? $_GET['error'] : '';
$errorMsg = '';
if ($error === 'invalid_url') {
    $errorMsg = 'Invalid URL. Please enter a valid YouTube video URL.';
} elseif ($error === 'channel_id_not_found') {
    $errorMsg = 'YouTube Channel ID not found.';
}
?>
<!doctype html>
<html lang="en">

<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>YouTube Sucks</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-QWTKZyjpPEjISv5WaRU9OFeRpok6YctnYmDr5pNlyT2bRjXh0JMhjY6hW+ALEwIH" crossorigin="anonymous">
</head>

<body>
    <div class="row mt-3">
        <div class="col-12 col-md-8 col-lg-4 offset-md-2 offset-lg-4">
            <div class="card">
                <div class="card-header">
                    <h3>Fix YouTube Play All</h3>
                </div>
                <div class="card-body">
                    <?php if ($errorMsg !== '') : ?>
                        <div class="alert alert-danger" role="alert">
                            <?php echo $errorMsg; ?>
                        </div>
                    <?php endif; ?>
                    <form method="post">
                        <input type="hidden" name="csrf_token" value="<?php echo htmlspecialchars($_SESSION['csrf_token']); ?>">
                        <div class="mb-3">
                            <label for="search" class="form-label">Enter a link to one video from the channel you want to play</label>
                            <input type="text" class="form-control" id="video" name="video" autofocus>
                        </div>
                        <button type="submit" class="btn btn-primary">Go</button>
                    </form>
                </div>
            </div>
            <p class="text-center pt-3">
                Want to host your own version of this? Check out the <a href="https://github.com/mudmin/AnotherMaker/tree/master/yt_sucks">Get the Code</a>.
            </p>
            <p class="text-center pt-2">
                If you appreciate my work, feel free to donate at <a href="https://userspice.com/donate/">https://userspice.com/donate/</a>.
            </p>
        </div>
    </div>

    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js" integrity="sha384-YvpcrYf0tY3lHB60NNkmXc5s9fDVZLESaAA55NDzOxhy9GkcIdslK1eN7N6jIeHz" crossorigin="anonymous"></script>
</body>

</html>