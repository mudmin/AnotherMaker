<?php
header('Content-Type: application/json');
foreach (glob('kicad_functions/*.php') as $filename) {
    include $filename;
}

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    echo json_encode(['error' => 'Invalid request method.']);
    exit;
}

if (!isset($_POST['cards']) || !is_array($_POST['cards'])) {
    echo json_encode(['error' => 'No card data received.']);
    exit;
}

$libSymbolsAll = "";
$instancesAll  = "";
$othersAll     = "";
$allCanCopy    = true;

foreach ($_POST['cards'] as $card) {
    $type = $card['type'];
    $logicFile = __DIR__ . "/kicad_logic/{$type}_logic.php";
    if (file_exists($logicFile)) {
        include_once $logicFile;
        $func = "generate_{$type}_logic";
        if (function_exists($func)) {
            $res = call_user_func($func, $card);
            $libSymbolsAll .= $res['library_symbols'] ?? "";
            $instancesAll  .= $res['instances']       ?? "";
            $othersAll     .= $res['others']          ?? "";
            if (isset($res['can_direct_copy']) && !$res['can_direct_copy']) {
                $allCanCopy = false;
            }
        }
    }
}

$fullOutput  = "(kicad_sch\n";
$fullOutput .= "\t(version 20231120)\n";
$fullOutput .= "\t(generator \"card-generator\")\n";
$fullOutput .= "\t(uuid \"" . generate_uuid() . "\")\n";
$fullOutput .= "\t(paper \"A4\")\n\n";
$fullOutput .= "\t(lib_symbols\n";
$fullOutput .= $libSymbolsAll . "\n";
$fullOutput .= "\t)\n\n";
$fullOutput .= $instancesAll . "\n";
$fullOutput .= $othersAll . "\n";
$fullOutput .= "\t(sheet_instances\n";
$fullOutput .= "\t\t(path \"/\"\n\t\t\t(page \"1\")\n\t\t)\n";
$fullOutput .= "\t)\n";
$fullOutput .= ")\n";

$filename = "generated_" . microtime(true) . ".kicad_sch";
$dynamicSnippet = $libSymbolsAll . "\n" . $instancesAll . "\n" . $othersAll;

echo json_encode([
    'fullOutput'      => $fullOutput,
    'dynamicOutput'   => $dynamicSnippet,
    'filename'        => $filename,
    'allCanCopy'      => $allCanCopy
]);
