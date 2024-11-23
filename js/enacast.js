(function ($) {
  // Backdrop.behaviors.ajaxpagination = {
  //   attach: function (context, settings) {
  //     $('ul.pager li a').click(function () {
  //       var href = $(this).attr('href');
  //       if (href && href != '#') { // Remove active Page onclick
  //         $.ajax({
  //           url: href,
  //           type: "GET",
  //           success: function (data) {
  //             // TODO: improve this to avoid retrieving the whole html
  //             var $loadedData = $('<div>').html(data);
  //             var content = $loadedData.find('#ajax-table-wrapper').html();
  //             $('#ajax-table-wrapper').html(content);
  //             Backdrop.attachBehaviors("#ajax-table-wrapper");
  //           }
  //         });
  //       }
  //       return false;
  //     });
  //   }
  // };

  Backdrop.behaviors.audioPlayer = {
    attach: function (context, settings) {
      // Check if audioPlayer setting exists before attempting to read startTime
      if (settings.audioPlayer) {
        let startTime = settings.audioPlayer.startTime;
        if (startTime && context === document) {
          jumpToMarker(startTime);
        }
      }
    }
  };

  window.jumpToMarker = function(time) {
    let audioElement = document.getElementById('enacast-podcast');
    audioElement.currentTime = time;
    // audioElement.play();
  }
})(jQuery);
