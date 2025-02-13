<?php
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // UUID generator function (version 4)
    function generate_uuid() {
        $data = random_bytes(16);
        $data[6] = chr((ord($data[6]) & 0x0f) | 0x40);
        $data[8] = chr((ord($data[8]) & 0x3f) | 0x80);
        return vsprintf('%s%s-%s-%s-%s-%s%s%s', str_split(bin2hex($data), 4));
    }
    
    $baseX = 30;  // Starting X coordinate for the first label set
    $baseY = 20;  // Fixed Y coordinate for each card's first label
    $cardGap = 40; // Horizontal gap between each label set
    
    // $globalX will track the current X coordinate for each card.
    $globalX = $baseX;
    
    $output = "(kicad_sch\n\t(version 20231120)\n\t(generator \"netlabel-generator\")\n";
    $output .= "\t(generator_version \"1.0\")\n\t(uuid \"" . generate_uuid() . "\")\n\t(paper \"A4\")\n\t(lib_symbols)\n";
    
    $filenameParts = [];
    $numCards = count($_POST['start_pin']);
    
    // Process each card (label set)
    for ($cardIndex = 0; $cardIndex < $numCards; $cardIndex++) {
         $start_pin = intval($_POST['start_pin'][$cardIndex]);
         $end_pin   = intval($_POST['end_pin'][$cardIndex]);
         $order     = $_POST['order'][$cardIndex];
         $prefix    = $_POST['prefix'][$cardIndex];
         $filter    = $_POST['filter'][$cardIndex];
         $spacing   = floatval($_POST['spacing'][$cardIndex]);
         
         // Build a descriptor for this card for the filename.
         $filenameParts[] = "pins{$start_pin}-{$end_pin}_{$prefix}_{$filter}_{$order}";
         
         // Build the list of pin numbers based on the range and filter.
         $numbers = [];
         for ($i = $start_pin; $i <= $end_pin; $i++) {
             if ($filter == 'all' || ($filter == 'odd' && $i % 2 == 1) || ($filter == 'even' && $i % 2 == 0)) {
                 $numbers[] = $i;
             }
         }
         if ($order == 'down') {
             $numbers = array_reverse($numbers);
         }
         // For each number in this card, generate a net label.
         foreach ($numbers as $index => $num) {
             $label = $prefix . $num;
             // Each label in a set is spaced vertically starting at $baseY.
             $y = $baseY + ($index * $spacing);
             $output .= "\t(global_label \"$label\"\n";
             $output .= "\t\t(shape input)\n";
             $output .= "\t\t(at $globalX " . number_format($y, 2, '.', '') . " 180)\n";
             $output .= "\t\t(fields_autoplaced yes)\n";
             $output .= "\t\t(effects (font (size 1.27 1.27)) (justify right))\n";
             $output .= "\t\t(uuid \"" . generate_uuid() . "\")\n";
             $output .= "\t)\n";
         }
         // Move the starting X coordinate for the next card to the right.
         $globalX += $cardGap;
    }
    
    $output .= "\t(sheet_instances\n";
    $output .= "\t\t(path \"/\"\n";
    $output .= "\t\t\t(page \"1\")\n";
    $output .= "\t\t)\n";
    $output .= "\t)\n";
    $output .= ")\n";
    
    $filenameDescriptor = implode("__", $filenameParts);
    $filename = "{$filenameDescriptor}." . microtime(true) . ".kicad_sch";
    $generated_text = htmlspecialchars($output);
} else {
    $generated_text = "";
    $filename = "";
}
?>
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>KiCad Lazy Label Generator by AnotherMaker</title>
  <!-- Bootstrap 5 CSS CDN -->
  <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
  <!-- jQuery CDN -->
  <script src="https://code.jquery.com/jquery-3.6.0.min.js"></script>
  <style>
    .card-header .deleteCardBtn { cursor: pointer; }
  </style>
</head>
<body>
<div class="container my-5">
  <h1 class="mb-4">KiCad Lazy Label Generator by AnotherMaker</h1>
  <p>Enter your parameters for your net labels.  You may create multiple sets in one schematic file.  When you are ready, click generate.  Download the schematic.  Open it in kicad. Select your labels, copy, and past them into your real schematic. The whole process takes less than a minute.</p>
  <p>If you appreciate my work, feel free to donate over at <a href="https://userspice.com/donate">https://userspice.com/donate</a>.</p>
  <form method="post" id="labelForm">
    <div id="cardContainer">
      <?php if ($_SERVER['REQUEST_METHOD'] === 'POST'): 
          $numCards = count($_POST['start_pin']);
          for ($i = 0; $i < $numCards; $i++): ?>
      <div class="card mb-3 cardItem">
        <div class="card-header d-flex justify-content-between align-items-center">
          <span>Label Set</span>
          <button type="button" class="btn btn-sm btn-danger deleteCardBtn">Delete</button>
        </div>
        <div class="card-body">
          <!-- Row for starting and ending pin -->
          <div class="row mb-2">
            <div class="col-md-6">
              <label class="form-label">Starting Pin</label>
              <input type="number" name="start_pin[]" class="form-control" value="<?php echo htmlspecialchars($_POST['start_pin'][$i]); ?>" required>
            </div>
            <div class="col-md-6">
              <label class="form-label">Ending Pin</label>
              <input type="number" name="end_pin[]" class="form-control" value="<?php echo htmlspecialchars($_POST['end_pin'][$i]); ?>" required>
            </div>
          </div>
          <!-- Row for counting order, prefix and number filter -->
          <div class="row mb-2">
            <div class="col-md-4">
              <label class="form-label">Counting Order</label>
              <select name="order[]" class="form-select">
                <option value="up" <?php if ($_POST['order'][$i] === 'up') echo 'selected'; ?>>Counting Up</option>
                <option value="down" <?php if ($_POST['order'][$i] === 'down') echo 'selected'; ?>>Counting Down</option>
              </select>
            </div>
            <div class="col-md-4">
              <label class="form-label">Prefix</label>
              <input type="text" name="prefix[]" class="form-control" value="<?php echo htmlspecialchars($_POST['prefix'][$i]); ?>" placeholder="e.g. PH_">
            </div>
            <div class="col-md-4">
              <label class="form-label">Number Filter</label>
              <select name="filter[]" class="form-select">
                <option value="all" <?php if ($_POST['filter'][$i] === 'all') echo 'selected'; ?>>All Numbers</option>
                <option value="odd" <?php if ($_POST['filter'][$i] === 'odd') echo 'selected'; ?>>Odd Only</option>
                <option value="even" <?php if ($_POST['filter'][$i] === 'even') echo 'selected'; ?>>Even Only</option>
              </select>
            </div>
          </div>
          <!-- Row for spacing -->
          <div class="row">
            <div class="col-md-6">
              <label class="form-label">Spacing (mm)</label>
              <input type="number" step="0.01" name="spacing[]" class="form-control" value="<?php echo htmlspecialchars($_POST['spacing'][$i]); ?>" required>
            </div>
          </div>
        </div>
      </div>
      <?php endfor; else: ?>
      <!-- Default card if no post data -->
      <div class="card mb-3 cardItem">
        <div class="card-header d-flex justify-content-between align-items-center">
          <span>Label Set</span>
          <button type="button" class="btn btn-sm btn-danger deleteCardBtn">Delete</button>
        </div>
        <div class="card-body">
          <!-- Row for starting and ending pin -->
          <div class="row mb-2">
            <div class="col-md-6">
              <label class="form-label">Starting Pin</label>
              <input type="number" name="start_pin[]" class="form-control" required>
            </div>
            <div class="col-md-6">
              <label class="form-label">Ending Pin</label>
              <input type="number" name="end_pin[]" class="form-control" required>
            </div>
          </div>
          <!-- Row for counting order, prefix and number filter -->
          <div class="row mb-2">
            <div class="col-md-4">
              <label class="form-label">Counting Order</label>
              <select name="order[]" class="form-select">
                <option value="up">Counting Up</option>
                <option value="down">Counting Down</option>
              </select>
            </div>
            <div class="col-md-4">
              <label class="form-label">Prefix</label>
              <input type="text" name="prefix[]" class="form-control" placeholder="e.g. PH_">
            </div>
            <div class="col-md-4">
              <label class="form-label">Number Filter</label>
              <select name="filter[]" class="form-select">
                <option value="all">All Numbers</option>
                <option value="odd">Odd Only</option>
                <option value="even">Even Only</option>
              </select>
            </div>
          </div>
          <!-- Row for spacing -->
          <div class="row">
            <div class="col-md-6">
              <label class="form-label">Spacing (mm)</label>
              <input type="number" step="0.01" name="spacing[]" class="form-control" value="2.54" required>
            </div>
          </div>
        </div>
      </div>
      <?php endif; ?>
    </div>
    <div class="mb-3">
      <button type="button" id="addCardBtn" class="btn btn-info">
        Add Label Set <span class="ms-1">+</span>
      </button>
    </div>
    <button type="submit" class="btn btn-primary">Generate</button>
  </form>
  
  <?php if($generated_text): ?>
  <hr>
  <h2>Generated Output</h2>
  <div class="mb-3">
    <textarea id="output" class="form-control" rows="20"><?php echo $output; ?></textarea>
  </div>
  <button id="copyBtn" class="btn btn-secondary">Copy to Clipboard</button>
  <a id="downloadLink" class="btn btn-success" href="data:text/plain;charset=utf-8,<?php echo rawurlencode($output); ?>" download="<?php echo $filename; ?>">Download .kicad_sch File</a>
  <?php endif; ?>
</div>
<script>
$(document).ready(function(){
  // Function to add a new card for label set input.
  function addCard() {
    var cardHtml = `
      <div class="card mb-3 cardItem">
        <div class="card-header d-flex justify-content-between align-items-center">
          <span>Label Set</span>
          <button type="button" class="btn btn-sm btn-danger deleteCardBtn">Delete</button>
        </div>
        <div class="card-body">
          <div class="row mb-2">
            <div class="col-md-6">
              <label class="form-label">Starting Pin</label>
              <input type="number" name="start_pin[]" class="form-control" required>
            </div>
            <div class="col-md-6">
              <label class="form-label">Ending Pin</label>
              <input type="number" name="end_pin[]" class="form-control" required>
            </div>
          </div>
          <div class="row mb-2">
            <div class="col-md-4">
              <label class="form-label">Counting Order</label>
              <select name="order[]" class="form-select">
                <option value="up">Counting Up</option>
                <option value="down">Counting Down</option>
              </select>
            </div>
            <div class="col-md-4">
              <label class="form-label">Prefix</label>
              <input type="text" name="prefix[]" class="form-control" placeholder="e.g. PH_">
            </div>
            <div class="col-md-4">
              <label class="form-label">Number Filter</label>
              <select name="filter[]" class="form-select">
                <option value="all">All Numbers</option>
                <option value="odd">Odd Only</option>
                <option value="even">Even Only</option>
              </select>
            </div>
          </div>
          <div class="row">
            <div class="col-md-6">
              <label class="form-label">Spacing (mm)</label>
              <input type="number" step="0.01" name="spacing[]" class="form-control" value="2.54" required>
            </div>
          </div>
        </div>
      </div>
    `;
    $('#cardContainer').append(cardHtml);
  }
  
  // Add new card when clicking the Add button.
  $('#addCardBtn').click(function(){
    addCard();
  });
  
  // Delete card when clicking the delete button.
  // If no card remains, automatically add a blank one.
  $(document).on('click', '.deleteCardBtn', function(){
    $(this).closest('.cardItem').remove();
    if ($('#cardContainer .cardItem').length === 0) {
      addCard();
    }
  });
  
  // Copy-to-clipboard functionality.
  $('#copyBtn').click(function(){
    $('#output').select();
    document.execCommand('copy');
    alert('Copied to clipboard');
  });
});
</script>
<?php if($generated_text): ?>
<script>
// Scroll to the bottom of the page after generation
$(document).ready(function(){
  $('html, body').animate({ scrollTop: $(document).height() }, 1000);
});
</script>
<?php endif; ?>
<!-- Bootstrap 5 Bundle CDN -->
<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/js/bootstrap.bundle.min.js"></script>
</body>
</html>
