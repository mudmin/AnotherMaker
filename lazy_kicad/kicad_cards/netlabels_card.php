<div class="card mb-3 cardItem" data-index="__INDEX__">
  <div class="card-header">
    <div class="row">
        <div class="col-6">
            <div class="card-title">Net Labels</div>
        </div>
        <div class="col-6 text-end">
            <button type="button" class="btn btn-sm btn-secondary cloneCardBtn">Clone</button>
            <button type="button" class="btn btn-sm btn-danger deleteCardBtn">Delete</button>
        </div>
    </div>

  </div>
  <div class="card-body">
    <!-- Hidden field to identify card type -->
    <input type="hidden" name="cards[__INDEX__][type]" value="netlabels">
    
    <div class="row mb-2">
      <div class="col-md-6">
        <label class="form-label">Starting Pin</label>
        <input type="number" name="cards[__INDEX__][start_pin]" class="form-control" required>
      </div>
      <div class="col-md-6">
        <label class="form-label">Ending Pin</label>
        <input type="number" name="cards[__INDEX__][end_pin]" class="form-control" required>
      </div>
    </div>
    
    <div class="row mb-2">
      <div class="col-md-4">
        <label class="form-label">Counting Order</label>
        <select name="cards[__INDEX__][order]" class="form-select">
          <option value="up">Counting Up</option>
          <option value="down">Counting Down</option>
        </select>
      </div>
      <div class="col-md-4">
        <label class="form-label">Prefix</label>
        <input type="text" name="cards[__INDEX__][prefix]" class="form-control" placeholder="e.g. PH_">
      </div>
      <div class="col-md-4">
        <label class="form-label">Number Filter</label>
        <select name="cards[__INDEX__][filter]" class="form-select">
          <option value="all">All Numbers</option>
          <option value="odd">Odd Only</option>
          <option value="even">Even Only</option>
        </select>
      </div>
    </div>
    
    <div class="row mb-2">
      <div class="col-md-6">
        <label class="form-label">Spacing (mm)</label>
        <input type="number" step="0.01" name="cards[__INDEX__][spacing]" class="form-control" value="2.54" required>
      </div>
      <div class="col-md-6">
        <label class="form-label">Label Style</label>
        <select name="cards[__INDEX__][label_style]" class="form-select">
          <option value="global" selected>Global (default)</option>
          <option value="local">Local</option>
          <option value="hierarchical">Hierarchical</option>
        </select>
      </div>
    </div>
  </div>
</div>
