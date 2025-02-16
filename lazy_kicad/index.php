<?php
//include all php files in the functions folder
foreach (glob('kicad_functions/*.php') as $filename) {
    include $filename;
}
$cards = getAvailableCards();

?>
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <title>KiCad Schematic Generator</title>

    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">

    <script src="https://code.jquery.com/jquery-3.6.0.min.js"></script>
    <style>
        .card-header .deleteCardBtn {
            cursor: pointer;
        }

        #toast {
            position: fixed;
            bottom: 20px;
            right: 20px;
            min-width: 200px;
            z-index: 1050;
        }
    </style>
</head>

<body>
    <div class="container my-5">
        <h1>KiCad Schematic Generator v1.1</h1>
        <p>The goal of this project is to do more than just labels. You can do that but just hitting the insert key. The goal is to make it quick and easy to programmatically create connected symbols, (custom) footprints and connected labels. More to come, but I did a little update for v1.1 that gives you more options and allows you to paste your creations (when compatible) directly into kicad without downloading files.</p>
        <p>If you appreciate my work, feel free to donate over at <a href="https://userspice.com/donate">https://userspice.com/donate</a>.
        Developers should note that as of v1.1 you can add files in the logic and card folders to extend the project.    <small><a href="_old_versions/v1.0/index.php">Click here to use the old version</a>
        </p>

        <p>Select and configure your settings below.</p>

        
        <!-- Card type selection -->
        <div class="mb-3">
            <label for="cardTypeSelect" class="form-label">Select Type:</label>
            <div class="input-group">
                <select id="cardTypeSelect" class="form-select">
                    <?php foreach ($cards as $cardKey => $cardLabel) : ?>
                        <option value="<?php echo $cardKey; ?>"><?php echo $cardLabel; ?></option>
                    <?php endforeach; ?>
                </select>
                <button type="button" class="btn btn-info addCardBtn">Add Card</button>
            </div>

        </div>

        <form id="cardForm">
            <div id="cardContainer">
                <!-- Card fragments (forms) will be loaded here via AJAX -->
            </div>
            <div class="mb-3">
                <button type="button" class="btn btn-info addCardBtn">Add Card</button>
            </div>
            <button type="submit" class="btn btn-primary">Generate</button>
        </form>

        <!-- Output section -->
        <div id="resultSection" class="mt-4" style="display:none;">
            <h2>Generated Output</h2>
            <div class="mb-2">
                <textarea id="output" class="form-control" rows="20" readonly></textarea>
            </div>
            <div class="mb-2">
                <a id="downloadLink" class="btn btn-success" href="#" download="">Download .kicad_sch File</a>
            </div>
        </div>
    </div>

    <!-- Toast notification -->
    <div id="toast" class="alert alert-info" role="alert" style="display: none;">
        Dynamic fields auto-copied to clipboard! You can paste them directly into KiCad.
    </div>

    <!-- Bootstrap 5 Bundle CDN -->
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/js/bootstrap.bundle.min.js"></script>
    <script>
        $(document).ready(function() {
            let cardIndex = 0;

            // Load a card fragment from the kicad_cards folder.
            function loadCard(cardType) {
                $.ajax({
                    url: 'kicad_cards/' + cardType + '_card.php',
                    method: 'GET',
                    success: function(data) {
                        // Replace __INDEX__ with a running index.
                        let cardHtml = data.replace(/__INDEX__/g, cardIndex);
                        $('#cardContainer').append(cardHtml);
                        cardIndex++;
                    },
                    error: function() {
                        alert('Error loading card template for ' + cardType);
                    }
                });
            }

            $('.addCardBtn').click(function() {
                let cardType = $('#cardTypeSelect').val();
                loadCard(cardType);
            });


            $(document).on('click', '.deleteCardBtn', function() {
                $(this).closest('.cardItem').remove();
            });


            $('#cardForm').submit(function(e) {
                e.preventDefault();
                const formData = $(this).serialize();

                $.ajax({
                    url: 'process.php',
                    type: 'POST',
                    data: formData,
                    dataType: 'json',
                    success: function(response) {
                        if (response.error) {
                            alert(response.error);
                            return;
                        }

                        // Populate the output textarea with the full schematic text
                        $('#output').val(response.fullOutput);

                        // Set up the download link
                        const downloadHref = 'data:text/plain;charset=utf-8,' + encodeURIComponent(response.fullOutput);
                        $('#downloadLink')
                            .attr('href', downloadHref)
                            .attr('download', response.filename);

                        // Show the result section
                        $('#resultSection').show();

                        // Handle copy-to-clipboard logic
                        if (response.allCanCopy) {
                            copyTextToClipboard(response.dynamicOutput);
                            $('#toast').text('Snippet copied! You can paste it directly into KiCad.').fadeIn();
                        } else {
                            $('#toast').text('Symbol definitions cannot be pasted directly. Please download the schematic.').fadeIn();
                        }

                        // Hide the toast after 15 seconds
                        setTimeout(function() {
                            $('#toast').fadeOut();
                        }, 15000);
                    },
                    error: function() {
                        alert('An error occurred while processing the form.');
                    }
                });
            });

            $(document).on('click', '.cloneCardBtn', function() {
                // Clone the cardItem
                let $originalCard = $(this).closest('.cardItem');
                let $clone = $originalCard.clone();

                let oldIndex = $originalCard.attr('data-index');
                let newIndex = cardIndex++;
                $clone.attr('data-index', newIndex);
                $clone.find('[name]').each(function() {
                    let name = $(this).attr('name');

                    let newName = name.replace(oldIndex, newIndex);
                    $(this).attr('name', newName);
                });

                // Append the cloned card to the container
                $('#cardContainer').append($clone);
            });




            function copyTextToClipboard(text) {
                let tempTextarea = $('<textarea>');
                $('body').append(tempTextarea);
                tempTextarea.val(text).select();
                document.execCommand('copy');
                tempTextarea.remove();
            }
        });
    </script>
</body>

</html>