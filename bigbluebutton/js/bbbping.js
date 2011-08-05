// JavaScript Document
		//JQuery code STARTS
		$(document).ready(function() {
			$('#dynamic').html( htmlContent );
            
			$.jheartbeat.set({
              url: Drupal.settings.pingURL,
              delay: 2500
              }, function() {
                 mycallback();
              });

		} );
		//JQuery code ENDS

	    function mycallback() {
           // Not elegant, but works around a bug in IE8
           var isMeeting = ($("#HeartBeatDIV").text().search("true")  > 0 );
           if ( isMeeting ) {
              window.location = Drupal.settings.joinURL;
           } else {
              //window.location.reload(true);
           }
        }
