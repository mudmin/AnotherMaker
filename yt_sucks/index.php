<?php
$isHttps = (!empty($_SERVER['HTTPS']) && $_SERVER['HTTPS'] !== 'off')
    || (($_SERVER['HTTP_X_FORWARDED_PROTO'] ?? '') === 'https')
    || (($_SERVER['SERVER_PORT'] ?? '') == 443);

session_set_cookie_params([
    'lifetime' => 0,
    'path' => '/',
    'secure' => $isHttps,
    'httponly' => true,
    'samesite' => 'Lax',
]);
session_start();

header('X-Content-Type-Options: nosniff');
header('X-Frame-Options: DENY');
header('Referrer-Policy: strict-origin-when-cross-origin');
header(
    "Content-Security-Policy: default-src 'self'; "
    . "img-src 'self' https://img.youtube.com data:; "
    . "style-src 'self' 'unsafe-inline' https://cdn.jsdelivr.net; "
    . "script-src 'self' https://cdn.jsdelivr.net; "
    . "connect-src 'self' https://cdn.jsdelivr.net; "
    . "form-action 'self' https://www.youtube.com https://youtube.com https://m.youtube.com https://music.youtube.com https://youtu.be; "
    . "base-uri 'self'; "
    . "frame-ancestors 'none'"
);

$version = "3.0.2";
$date = "April 23, 2026";

/**
 * Verifies a URL's host is on the YouTube allowlist.
 * @param string $url
 * @return bool
 */
function is_youtube_url($url)
{
    $host = parse_url($url, PHP_URL_HOST);
    if (!is_string($host) || $host === '') {
        return false;
    }
    $scheme = parse_url($url, PHP_URL_SCHEME);
    if (strtolower((string)$scheme) !== 'https') {
        return false;
    }
    $allowed = ['www.youtube.com', 'youtube.com', 'm.youtube.com', 'music.youtube.com', 'youtu.be'];
    return in_array(strtolower($host), $allowed, true);
}

/**
 * Fetches the content of a remote URL using cURL.
 * @param string $url The URL to fetch.
 * @return string|false The content of the URL on success, or false on failure.
 */
function curl_get_contents($url) {
    if (!is_youtube_url($url)) {
        return false;
    }

    $maxBytes = 5 * 1024 * 1024; // 5 MB cap
    $buffer = '';
    $tooLarge = false;

    $ch = curl_init();

    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, false);
    curl_setopt($ch, CURLOPT_HEADER, 0);
    curl_setopt($ch, CURLOPT_TIMEOUT, 10);
    curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 5);
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, true);
    curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 2);
    curl_setopt($ch, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
    curl_setopt($ch, CURLOPT_WRITEFUNCTION, function ($ch, $data) use (&$buffer, &$tooLarge, $maxBytes) {
        $buffer .= $data;
        if (strlen($buffer) > $maxBytes) {
            $tooLarge = true;
            return 0; // abort
        }
        return strlen($data);
    });

    // Manual redirect handling, re-validating host on each hop.
    $currentUrl = $url;
    $maxRedirects = 5;
    for ($i = 0; $i <= $maxRedirects; $i++) {
        curl_setopt($ch, CURLOPT_URL, $currentUrl);
        $buffer = '';
        $tooLarge = false;
        curl_exec($ch);

        if ($tooLarge || curl_errno($ch)) {
            curl_close($ch);
            return false;
        }

        $code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        if ($code >= 300 && $code < 400) {
            $next = curl_getinfo($ch, CURLINFO_REDIRECT_URL);
            if (!is_string($next) || $next === '' || !is_youtube_url($next)) {
                curl_close($ch);
                return false;
            }
            $currentUrl = $next;
            continue;
        }

        curl_close($ch);
        return $buffer;
    }

    curl_close($ch);
    return false;
}
// ---------------------------------

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

    // Get the URL input, stripping CRLF to prevent header injection.
    $url = is_string($_POST["video"] ?? null) ? $_POST["video"] : '';
    $url = str_replace(["\r", "\n", "\0"], '', $url);
    // Cap length to prevent absurdly long inputs.
    if (strlen($url) > 2048) {
        header("Location: " . $_SERVER['SCRIPT_NAME'] . "?error=invalid_url");
        exit();
    }

    if (!is_youtube_url($url)) {
        header("Location: " . $_SERVER['SCRIPT_NAME'] . "?error=invalid_url");
        exit();
    }

    //throw away anything after and including the first "&"
    $url = explode("&", $url)[0];

    $remote_content = curl_get_contents($url);

    if ($remote_content === false) {
        // cURL failed to fetch the page
        header("Location: " . $_SERVER['SCRIPT_NAME'] . "?error=fetch_failed");
        exit();
    }

    $content = htmlspecialchars($remote_content);


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

        // Determine the playlist prefix based on user choice
        // UU is for "uploads"
        // UULF is for "uploads, without shorts"
        $prefix = "UU";
        if (!isset($_POST['include_shorts']) || $_POST['include_shorts'] != '1') {
            $prefix = "UULF";
        }

        $modifiedChannelId = $prefix . $mostFrequentChannel;

        // Create the final URL
        $finalUrl = $url . "&list=" . $modifiedChannelId;

        // Redirect to the modified URL
        header("Location: $finalUrl");
        exit();
    }

    // Redirect with an error message if no valid channel IDs found
    header("Location: " . $_SERVER['SCRIPT_NAME'] . "?error=channel_id_not_found");
    exit();
}

// If there's an error, display the appropriate message
$error = isset($_GET['error']) ? $_GET['error'] : '';
$errorMsg = '';
if ($error === 'invalid_url') {
    $errorMsg = 'Invalid URL. Please enter a valid YouTube video URL.';
} elseif ($error === 'channel_id_not_found') {
    $errorMsg = 'YouTube Channel ID not found.';
} elseif ($error === 'fetch_failed') {
    $errorMsg = 'Failed to fetch content from the YouTube URL. This could be a temporary network issue.';
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
