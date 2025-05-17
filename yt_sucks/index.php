<?php
session_start();
$version = "2.0.0";
$date = "May 17, 2025";

// Generate CSRF token and store it in session
if (!isset($_SESSION['csrf_token'])) {
    $_SESSION['csrf_token'] = bin2hex(random_bytes(32));
}

if (!function_exists('dump')) {
    function dump($var)
    {
        if (is_array($var) || is_object($var)) {
            echo '<pre>';
            print_r($var);
            echo '</pre>';
        } else {
            echo $var;
        }
    }
}

if (!function_exists('dnd')) {
    function dnd($var)
    {

        dump($var);
        die();
    }
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
        header("Location: " . $_SERVER['PHP_SELF'] . "?error=invalid_url");
        exit();
    }
    
    //throw away anything after and including the first "&"
    $url = explode("&", $url)[0];
    $content = htmlspecialchars(file_get_contents($url));

    // Apply regex to capture channel IDs
    preg_match_all('/\/UC([\w-]+)(?="|\/)/', $content, $matches);

    // Filter out UC codes that are longer than 25 characters before any processing
    $validUCs = array_filter($matches[1], function ($uc) {
        return strlen($uc) <= 25;
    });

    // Count the occurrences of each unique valid UC code
    $channelCounts = array_count_values($validUCs);

    // Check if there are any channel IDs found
    if (!empty($channelCounts)) {
        // Get the most frequent channel ID
        $mostFrequentChannel = array_keys($channelCounts, max($channelCounts))[0];

        // Modify the channel ID
        $modifiedChannelId = "UU" . $mostFrequentChannel;
        
        // If include_shorts is NOT checked, use UULF instead of UU to filter out shorts
        if (!isset($_POST['include_shorts']) || $_POST['include_shorts'] != '1') {
            $modifiedChannelId = "UULF" . $mostFrequentChannel;
        }

        // Create the final URL
        $finalUrl = $url . "&list=" . $modifiedChannelId;
        
        // If start_at_beginning is checked, add index=1
        // if (isset($_POST['start_at_beginning']) && $_POST['start_at_beginning'] == '1') {
        //     $finalUrl .= "&index=1";
        // }

        // Redirect to the modified URL
        header("Location: $finalUrl");
        exit();
    }

    // Redirect with an error message if no valid channel IDs found
    header("Location: " . $_SERVER['PHP_SELF'] . "?error=channel_id_not_found");
    exit();
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
                    <small>
                        Version: <?php echo $version; ?> | Updated: <?php echo $date; ?>
                    </small>
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
                            <input type="text" class="form-control" id="video" name="video" autofocus required>
                            <div class="input-group pt-2 pb-1">
                                <input type="checkbox"
                                    class="form-check-input"
                                    id="include_shorts" name="include_shorts" value="1">
                                <label for="include_shorts" class="ms-2">
                                    Include YouTube Shorts
                                </label>
                            </div>
                            <!-- <div class="input-group pt-2 pb-1">

                                <input type="checkbox" name="start_at_beginning" class="form-check-input" id="start_at_beginning" value="1">
                                <label for="start_at_beginning" class="ms-2">Start at the beginning of the channel</label>
                            </div> -->

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