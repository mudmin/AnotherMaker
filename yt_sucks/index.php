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

$version = "3.1.0";
$date = "October 18, 2026";

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
 * @param string|null $postJson Optional JSON body; when given, the request is a POST.
 * @return string|false The content of the URL on success, or false on failure.
 */
function curl_get_contents($url, $postJson = null) {
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
    if ($postJson !== null) {
        curl_setopt($ch, CURLOPT_POSTFIELDS, $postJson);
        curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Type: application/json']);
    }
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

/**
 * Normalizes whatever the user pasted (watch, youtu.be, shorts, live, embed,
 * @handle, /channel/, /c/, /user/ links, with or without https://) into
 * either a video ID or a channel.
 * @param string $input
 * @return array|false ['video_id' => ?string, 'channel_id' => ?string, 'channel_url' => ?string]
 */
function parse_youtube_input($input)
{
    $input = trim($input);
    if (!preg_match('~^[a-z][a-z0-9+.-]*://~i', $input)) {
        $input = 'https://' . $input;
    }
    $input = preg_replace('~^http://~i', 'https://', $input);
    if (!is_youtube_url($input)) {
        return false;
    }

    $host = strtolower(parse_url($input, PHP_URL_HOST));
    $segments = array_values(array_filter(explode('/', (string)parse_url($input, PHP_URL_PATH)), 'strlen'));
    parse_str((string)parse_url($input, PHP_URL_QUERY), $query);

    $result = ['video_id' => null, 'channel_id' => null, 'channel_url' => null];
    $first = $segments[0] ?? '';

    if ($host === 'youtu.be') {
        $result['video_id'] = $first;
    } elseif ($first === 'watch') {
        $result['video_id'] = is_string($query['v'] ?? null) ? $query['v'] : '';
    } elseif (in_array($first, ['shorts', 'live', 'embed', 'v'], true)) {
        $result['video_id'] = $segments[1] ?? '';
    } elseif ($first === 'channel' && preg_match('/^UC[\w-]{22}$/', $segments[1] ?? '')) {
        $result['channel_id'] = $segments[1];
    } elseif ($first !== '' && $first[0] === '@') {
        $result['channel_url'] = 'https://www.youtube.com/' . rawurlencode(rawurldecode($first));
    } elseif (in_array($first, ['c', 'user'], true) && isset($segments[1])) {
        $result['channel_url'] = 'https://www.youtube.com/' . $first . '/' . rawurlencode(rawurldecode($segments[1]));
    }

    if ($result['video_id'] !== null && !preg_match('/^[\w-]{11}$/', $result['video_id'])) {
        return false;
    }
    if ($result['video_id'] === null && $result['channel_id'] === null && $result['channel_url'] === null) {
        return false;
    }
    return $result;
}

/**
 * Finds the channel ID on a fetched YouTube page.
 * @param string $html Raw page HTML
 * @return string|null The UC... channel ID
 */
function find_channel_id($html)
{
    // Video pages: the owner of the video itself, not a recommended/featured channel.
    $pos = strpos($html, '"videoDetails":{');
    if ($pos !== false && preg_match('/"channelId":"(UC[\w-]{22})"/', $html, $m, 0, $pos)) {
        return $m[1];
    }
    // Channel pages.
    if (preg_match('/"externalId":"(UC[\w-]{22})"/', $html, $m)
        || preg_match('/<meta itemprop="identifier" content="(UC[\w-]{22})"/', $html, $m)) {
        return $m[1];
    }

    // Fallback: the most frequent /UC... reference on the page (the original heuristic).
    preg_match_all('/\/UC([\w-]+)(?="|\/)/', htmlspecialchars($html), $matches);
    $validUCs = array_filter($matches[1], function ($uc) {
        return strlen($uc) <= 25;
    });
    $channelCounts = array_count_values($validUCs);
    if (empty($channelCounts)) {
        return null;
    }
    return 'UC' . array_keys($channelCounts, max($channelCounts))[0];
}

/**
 * Yields every array node in a decoded YouTube JSON blob, depth first.
 */
function yt_nodes($node)
{
    if (!is_array($node)) {
        return;
    }
    yield $node;
    foreach ($node as $child) {
        if (is_array($child)) {
            yield from yt_nodes($child);
        }
    }
}

/**
 * Returns the first value stored under $key anywhere inside $node.
 */
function yt_find($node, $key)
{
    foreach (yt_nodes($node) as $n) {
        if (array_key_exists($key, $n)) {
            return $n[$key];
        }
    }
    return null;
}

/**
 * Pulls video IDs and the next continuation token out of a channel's videos
 * grid (either the initial page data or a browse API response).
 * @return array [string[] $ids, ?string $nextToken]
 */
function yt_grid_page($data)
{
    $ids = [];
    $next = null;
    foreach (yt_nodes($data) as $n) {
        if (isset($n['richItemRenderer'])) {
            $id = yt_find($n['richItemRenderer'], 'videoId');
            if (is_string($id) && !in_array($id, $ids, true)) {
                $ids[] = $id;
            }
        } elseif ($next === null && isset($n['continuationItemRenderer'])) {
            // The first one belongs to the videos grid; later ones are other page sections.
            $token = yt_find($n['continuationItemRenderer'], 'continuationCommand');
            $next = is_string($token['token'] ?? null) ? $token['token'] : null;
        }
    }
    return [$ids, $next];
}

/**
 * Turns a "Latest" sort-chip token from a channel's Videos tab into an
 * "Oldest" one. Channels with memberships show Latest / Members only / Public
 * chips instead of Latest / Popular / Oldest, but the browse API still accepts
 * the oldest sort; it's one byte in the token's nested params
 * (sort: 4 = latest, 2 = popular, 5 = oldest).
 * @param string $token URL-encoded, URL-safe base64 protobuf
 * @return string|null The oldest-sort token, or null if $token isn't a Latest chip
 */
function oldest_sort_token($token)
{
    $outer = base64_decode(strtr(rawurldecode($token), '-_', '+/'), true);
    // The params field (protobuf field 3) is a base64 string running to the end of the token.
    if ($outer === false || !preg_match('/\x1a([\x01-\x7f])([A-Za-z0-9%+\/=_-]+)$/', $outer, $m) || strlen($m[2]) !== ord($m[1])) {
        return null;
    }
    $params = base64_decode(rawurldecode($m[2]), true);
    if ($params === false || substr($params, -2) !== "\x18\x04") {
        return null;
    }
    $newParams = rawurlencode(base64_encode(substr($params, 0, -1) . "\x05"));
    if (strlen($newParams) !== strlen($m[2])) {
        return null;
    }
    return rawurlencode(strtr(base64_encode(substr($outer, 0, -strlen($newParams)) . $newParams), '+/', '-_'));
}

/**
 * Builds a channel's long-form uploads in oldest-first order, far enough to
 * cover the request. Progress is cached per channel so large channels can be
 * walked across several requests.
 *
 * YouTube has no oldest-first playlist, so this sorts the channel's Videos tab
 * oldest-first and pages through it via the internal browse API.
 *
 * @param string $channelId UC... channel ID
 * @param string|null $startVideoId Video that must be included (null = start at the very first)
 * @param int $count How many videos are wanted from the start point
 * @return array|false ['ids' => string[], 'complete' => bool], or false on failure
 */
function fetch_oldest_ids($channelId, $startVideoId, $count)
{
    $maxPages = 40;
    $timeBudget = 30;
    $cacheTtl = 86400;
    $startedAt = time();

    $cacheDir = __DIR__ . '/cache';
    $cacheFile = $cacheDir . '/oldest_' . $channelId . '.php';
    $state = null;
    if (is_file($cacheFile) && filemtime($cacheFile) > time() - $cacheTtl) {
        $cached = json_decode(preg_replace('/^<\?php die\(\);\?>\n/', '', (string)file_get_contents($cacheFile)), true);
        if (is_array($cached) && isset($cached['ids'], $cached['client_version']) && is_array($cached['ids'])) {
            $state = $cached;
        }
    }

    if ($state === null) {
        $html = curl_get_contents('https://www.youtube.com/channel/' . $channelId . '/videos?hl=en');
        if ($html === false) {
            return false;
        }
        $start = strpos($html, 'var ytInitialData = ');
        $end = $start === false ? false : strpos($html, ';</script>', $start);
        if ($end === false || !preg_match('/"INNERTUBE_CLIENT_VERSION":"([^"]+)"/', $html, $ver)) {
            return false;
        }
        $data = json_decode(substr($html, $start + 20, $end - $start - 20), true);
        if (!is_array($data)) {
            return false;
        }

        $state = ['ids' => [], 'next' => null, 'complete' => false, 'reverse' => false, 'client_version' => $ver[1]];
        foreach (yt_nodes($data) as $n) {
            if (isset($n['chipViewModel'])) {
                $token = yt_find($n['chipViewModel'], 'continuationCommand');
                $state['next'] = is_string($token['token'] ?? null) ? oldest_sort_token($token['token']) : null;
                if ($state['next'] !== null) {
                    break;
                }
            }
        }
        if ($state['next'] === null) {
            // Small channels get no sort chips: walk newest-first and flip at the end.
            // Parse only the videos grid; other page sections carry unrelated continuations.
            [$state['ids'], $state['next']] = yt_grid_page(yt_find($data, 'richGridRenderer') ?? []);
            $state['reverse'] = true;
            $state['complete'] = $state['next'] === null;
        }
    }

    $satisfied = function () use (&$state, $startVideoId, $count) {
        if ($state['complete']) {
            return true;
        }
        if ($state['reverse']) {
            return false;
        }
        $idx = $startVideoId === null ? 0 : array_search($startVideoId, $state['ids'], true);
        return $idx !== false && count($state['ids']) - $idx >= $count;
    };

    for ($page = 0; $page < $maxPages && !$satisfied() && time() - $startedAt < $timeBudget; $page++) {
        $body = json_encode([
            'context' => ['client' => ['clientName' => 'WEB', 'clientVersion' => $state['client_version'], 'hl' => 'en']],
            'continuation' => $state['next'],
        ]);
        $response = json_decode((string)curl_get_contents('https://www.youtube.com/youtubei/v1/browse?prettyPrint=false', $body), true);
        if (!is_array($response)) {
            break;
        }
        [$ids, $next] = yt_grid_page($response);
        if (empty($ids)) {
            // An empty page means a bad token or a format change, not the end of the channel.
            break;
        }
        $state['ids'] = array_values(array_unique(array_merge($state['ids'], $ids)));
        $state['next'] = $next;
        $state['complete'] = $next === null;
    }

    if ($state['reverse'] && !$state['complete']) {
        return false;
    }
    if ($state['reverse']) {
        $state['ids'] = array_reverse($state['ids']);
        $state['reverse'] = false;
    }

    if (!empty($state['ids']) && (is_dir($cacheDir) || @mkdir($cacheDir, 0775))) {
        $tmp = $cacheFile . '.' . bin2hex(random_bytes(4));
        if (@file_put_contents($tmp, "<?php die();?>\n" . json_encode($state)) !== false) {
            @rename($tmp, $cacheFile);
        }
    }

    return ['ids' => $state['ids'], 'complete' => $state['complete']];
}

// Play-order choices: [label, playlist prefix]. Prefixes turn a UC... channel ID
// into one of YouTube's auto-generated playlists; 'oldest' modes are built by hand.
$modes = [
    'newest'        => ['Newest first', 'UULF'],
    'newest_shorts' => ['Newest first, include Shorts', 'UU'],
    'oldest'        => ['Oldest first', null],
    'oldest_from'   => ['Oldest first, starting at this video', null],
    'popular'       => ['Most popular', 'UULP'],
    'shorts'        => ['Shorts only', 'UUSH'],
    'live'          => ['Live streams only', 'UULV'],
];
$oldestBatchSize = 50; // YouTube's cap for watch_videos temporary playlists
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

    // Nothing below needs the session; release its lock before the slow YouTube fetches.
    session_write_close();

    $mode = is_string($_POST['mode'] ?? null) && isset($modes[$_POST['mode']]) ? $_POST['mode'] : 'newest';
    $videoId = null;

    // Send errors back with the chosen mode (and video, once validated) so a retry is one click.
    $fail = function ($error) use (&$mode, &$videoId) {
        $query = ['error' => $error, 'mode' => $mode];
        if ($videoId) {
            $query['v'] = $videoId;
        }
        header("Location: " . $_SERVER['SCRIPT_NAME'] . "?" . http_build_query($query));
        exit();
    };

    // Get the URL input, stripping CRLF to prevent header injection.
    $url = is_string($_POST["video"] ?? null) ? $_POST["video"] : '';
    $url = str_replace(["\r", "\n", "\0"], '', $url);
    // Cap length to prevent absurdly long inputs.
    if (strlen($url) > 2048) {
        $fail('invalid_url');
    }

    $input = parse_youtube_input($url);
    if ($input === false) {
        $fail('invalid_url');
    }
    $videoId = $input['video_id'];
    $videoUrl = $videoId ? 'https://www.youtube.com/watch?v=' . $videoId : null;

    $channelId = $input['channel_id'];
    if ($channelId === null) {
        $remote_content = curl_get_contents($videoUrl ?? $input['channel_url']);
        if ($remote_content === false) {
            // cURL failed to fetch the page
            $fail('fetch_failed');
        }
        $channelId = find_channel_id($remote_content);
        if ($channelId === null) {
            $fail('channel_id_not_found');
        }
    }
    $channelCode = substr($channelId, 2);

    if ($mode === 'oldest' || $mode === 'oldest_from') {
        // Oldest first: a temporary playlist of up to 50 video IDs, in order.
        $startId = ($mode === 'oldest_from') ? $videoId : null;
        $oldest = fetch_oldest_ids($channelId, $startId, $oldestBatchSize);
        if ($oldest === false || empty($oldest['ids'])) {
            $fail('oldest_failed');
        }
        $offset = $startId === null ? 0 : array_search($startId, $oldest['ids'], true);
        if ($offset === false) {
            $fail($oldest['complete'] ? 'oldest_not_in_list' : 'oldest_not_found_yet');
        }
        $batch = array_slice($oldest['ids'], $offset, $oldestBatchSize);
        header("Location: https://www.youtube.com/watch_videos?video_ids=" . implode(',', $batch));
        exit();
    }

    $playlistId = $modes[$mode][1] . $channelCode;
    if (($mode === 'newest' || $mode === 'newest_shorts') && $videoUrl) {
        // Start at the pasted video and keep going through the channel.
        $finalUrl = $videoUrl . "&list=" . $playlistId;
    } else {
        // Start at the top of the list (e.g. the most popular video).
        $finalUrl = "https://www.youtube.com/watch?list=" . $playlistId;
    }

    // Redirect to the modified URL
    header("Location: $finalUrl");
    exit();
}

// If there's an error, display the appropriate message
$error = isset($_GET['error']) ? $_GET['error'] : '';
$errorMsg = '';
if ($error === 'invalid_url') {
    $errorMsg = 'Invalid URL. Please enter a link to a YouTube video or channel.';
} elseif ($error === 'channel_id_not_found') {
    $errorMsg = 'YouTube Channel ID not found.';
} elseif ($error === 'fetch_failed') {
    $errorMsg = 'Failed to fetch content from the YouTube URL. This could be a temporary network issue.';
} elseif ($error === 'oldest_failed') {
    $errorMsg = 'Couldn\'t load this channel\'s oldest videos. Please try again, or pick a different play order.';
} elseif ($error === 'oldest_not_found_yet') {
    $errorMsg = 'Still searching this channel\'s history for that video. Big channels take a few tries, so press Go again to search further back.';
} elseif ($error === 'oldest_not_in_list') {
    $errorMsg = 'That video isn\'t in the channel\'s oldest-first list. Shorts and live streams aren\'t included.';
}

// Pre-fill the form after an error so the user can retry.
$selectedMode = is_string($_GET['mode'] ?? null) && isset($modes[$_GET['mode']]) ? $_GET['mode'] : 'newest';
$prefillUrl = is_string($_GET['v'] ?? null) && preg_match('/^[\w-]{11}$/', $_GET['v'])
    ? 'https://www.youtube.com/watch?v=' . $_GET['v'] : '';
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
                            <label for="video" class="form-label">Enter a link to one video from the channel you want to play (or the channel itself)</label>
                            <input type="text" class="form-control" id="video" name="video" value="<?php echo htmlspecialchars($prefillUrl); ?>" autofocus required>
                            <div class="pt-2 pb-1 d-flex align-items-center gap-2">
                                <label for="mode" class="mb-0">Play</label>
                                <select class="form-select form-select-sm" id="mode" name="mode">
                                    <?php foreach ($modes as $key => $m): ?>
                                        <option value="<?php echo $key; ?>"<?php echo $key === $selectedMode ? ' selected' : ''; ?>><?php echo htmlspecialchars($m[0]); ?></option>
                                    <?php endforeach; ?>
                                </select>
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
