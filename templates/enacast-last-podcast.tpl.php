<?php

/**
 * @file
 * Template file for the theming example text form.
 *
 * Available custom variables:
 * - $text_form: A string containing the pre-rendered form.
 * - $text_form_content: An array of form elements keyed by the element name.
 *
 * The default example below renders the entire form and its form elements in
 * a default order provided by Backdrop.
 *
 * Alternatively, you may print each form element in the order you desire,
 * adding any extra html markup you wish to decorate the form like this:
 *
 * <?php print $text_form_content['element_name']; ?>
 *
 * The following snippet will print the contents of the $text_form_content
 * array, hidden in the source of the page, for you to discover the individual
 * element names.
 *
 * <?php print '<!--' . print_r($text_form_content, TRUE) . '-->'; ?>
 */
// dpm($variables);
?>
<div class="card">
  <div class="card-body">
    <h5 class="card-title"><?php print $title ?></h5>
    <p class="card-text"><?php print $summary ?></p>
    <audio controls>
      <source src="<?php print $mp3_url; ?>" type="audio/mpeg">
      Your browser does not support the audio tag.
    </audio>
  </div>
</div>
