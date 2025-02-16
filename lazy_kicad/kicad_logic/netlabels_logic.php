<?php
function generate_netlabels_logic($card) {
    static $netlabelsX = 30;
    $baseY = 20;
    $cardGap = 40;

    $start_pin   = isset($card['start_pin']) ? intval($card['start_pin']) : 0;
    $end_pin     = isset($card['end_pin'])   ? intval($card['end_pin'])   : 0;
    $order       = isset($card['order'])     ? $card['order']            : 'up';
    $prefix      = isset($card['prefix'])    ? $card['prefix']           : '';
    $filter      = isset($card['filter'])    ? $card['filter']           : 'all';
    $spacing     = isset($card['spacing'])   ? floatval($card['spacing']): 2.54;
    $label_style = isset($card['label_style']) ? $card['label_style']     : 'global';

    // Net labels can be pasted directly into KiCad
    $canDirectCopy = true;

    $others = "";
    $nums = [];
    for ($i = $start_pin; $i <= $end_pin; $i++) {
        if ($filter == 'all' ||
            ($filter == 'odd' && $i % 2 == 1) ||
            ($filter == 'even' && $i % 2 == 0)) {
            $nums[] = $i;
        }
    }
    if ($order == 'down') $nums = array_reverse($nums);

    foreach ($nums as $idx => $n) {
        $label = $prefix . $n;
        $y = $baseY + ($idx * $spacing);
        $others .= generate_net_label($label, $netlabelsX, $y, $label_style);
    }
    $netlabelsX += $cardGap;

    return [
        'library_symbols' => '',
        'instances'       => '',
        'others'          => $others,
        'can_direct_copy' => $canDirectCopy
    ];
}
