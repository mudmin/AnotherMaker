<?php

/**
 * Generate a UUID version 4.
 */
function generate_uuid()
{
    $data = random_bytes(16);
    $data[6] = chr((ord($data[6]) & 0x0f) | 0x40);
    $data[8] = chr((ord($data[8]) & 0x3f) | 0x80);
    return vsprintf('%s%s-%s-%s-%s-%s%s%s', str_split(bin2hex($data), 4));
}

/**
 * Scan the kicad_cards folder for card templates and return an associative array.
 * The key is the card type (derived from the filename) and the value is a friendly label.
 */
function getAvailableCards()
{
    $cards = [];
    $files = glob('kicad_cards/*_card.php');
    // Predefined friendly labels for known types
    $friendlyLabels = [
        'netlabels' => 'Create Net Labels',
        'pinheader' => 'Create Pin Headers'
    ];
    foreach ($files as $file) {
        $basename = basename($file);
        // Remove the suffix _card.php to get card key
        $cardKey = str_replace('_card.php', '', $basename);
        if (isset($friendlyLabels[$cardKey])) {
            $cards[$cardKey] = $friendlyLabels[$cardKey];
        } else {
            // Default label if none is predefined
            $cards[$cardKey] = 'Create ' . ucwords(str_replace('_', ' ', $cardKey));
        }
    }
    return $cards;
}

/**
 * Load the card logic file for a given card type.
 */
function loadCardLogic($cardType)
{
    $logicFile = "kicad_logic/{$cardType}_logic.php";
    if (file_exists($logicFile)) {
        require_once $logicFile;
        return true;
    }
    return false;
}


function generate_net_label($label, $x, $y, $style = 'global')
{
    // Generate a UUID for the label.
    $uuid = generate_uuid();
    switch ($style) {
        case 'local':
            // Example for a local label:
            $output  = "(label \"$label\" (at $x $y 0) (fields_autoplaced yes)\n";
            $output .= "  (effects (font (size 1.27 1.27)) (justify left bottom))\n";
            $output .= "  (uuid \"$uuid\")\n";
            $output .= ")\n";
            break;
        case 'hierarchical':
            $output  = "(hierarchical_label \"$label\" (shape input) (at $x $y 0) (fields_autoplaced yes)\n";
            $output .= "  (effects (font (size 1.27 1.27)) (justify left))\n";
            $output .= "  (uuid \"$uuid\")\n";

            $output .= "  (property \"Intersheetrefs\" \"\${INTERSHEET_REFS}\" (at " . ($x + 8) . " $y 0) (effects (font (size 1.27 1.27)) (justify left) (hide yes)))\n";
            $output .= ")\n";
            break;
        default:
            // Default (global) label style:
            $output  = "(global_label \"$label\" (shape input) (at $x $y 0) (fields_autoplaced yes)\n";
            $output .= "  (effects (font (size 1.27 1.27)) (justify left))\n";
            $output .= "  (uuid \"$uuid\")\n";
            // Add Intersheetrefs property for global labels:
            $output .= "  (property \"Intersheetrefs\" \"\${INTERSHEET_REFS}\" (at " . ($x + 8) . " $y 0) (effects (font (size 1.27 1.27)) (justify left) (hide yes)))\n";
            $output .= ")\n";
            break;
    }
    return $output;
}

function reorderCounterclockwise(array $leftPins, array $rightPins) : array
{
    $newLeftPins  = [];
    $newRightPins = [];

    // Combine them in an alternating sequence
    $iL = 0;
    $iR = 0;
    $max = max(count($leftPins), count($rightPins));
    $newPinNum = 1;

    for ($i = 0; $i < $max; $i++) {
        if ($iL < count($leftPins)) {
            $newLeftPins[] = $newPinNum++;
            $iL++;
        }
        if ($iR < count($rightPins)) {
            $newRightPins[] = $newPinNum++;
            $iR++;
        }
    }

    // Now $newLeftPins and $newRightPins hold the new numbering in a “zig-zag” style.
    return [$newLeftPins, $newRightPins];
}


function dump($var)
{
    echo '<pre>';
    var_dump($var);
    echo '</pre>';
}

function dnd($var)
{
    dump($var);
    die();
}
