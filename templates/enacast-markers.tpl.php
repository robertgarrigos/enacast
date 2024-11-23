<?php

/**
 * @file
 * Template to display the now playing block
 */

?>

<div class="enacast-markers">

  <?php foreach ($markers as $marker) { ?>
    <div class="enacast-marker">
    <button class="button" onclick="jumpToMarker(<?php print $marker->startsecond ?>)">Salta a <?php print $marker->descr; ?></button>
    <a href="<?php print $path . '?s=' . $marker->startsecond ?>"><img src="/<?php print $share_icon; ?>" alt="My Icon" class="share-icon" /></a>
    </div>
  <?php } ?>

</div>
