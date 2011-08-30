        function bigbluebutton_joinURL(){
            window.location = Drupal.settings.bigbluebutton_joinURL;
        }

        function bigbluebutton_callback() {
            // Not elegant, but works around a bug in IE8
            var isMeeting = ($("#HeartBeatDIV").text().search("true")  > 0 );
            if ( isMeeting ) {
                bigbluebutton_joinURL();
            }
        }

        function bigbluebutton_actionCall(url, action){
            action = (typeof action == 'undefined') ? 'publish' : action;
       
            if ( action == 'publish' || (action == 'delete' && confirm("Are you sure to delete this recording?") ) ) {
                if (window.XMLHttpRequest) {
                    req = new XMLHttpRequest();
                } else if (window.ActiveXObject) {

                    try {
                        req = new ActiveXObject("Msxml2.XMLHTTP");
                    } catch (e) {

                        try {
                            req = new ActiveXObject("Microsoft.XMLHTTP");
                        } catch (e) {}
                    }
                }

                var req = new XMLHttpRequest();
                req.open("GET", url, true);
                req.onreadystatechange = function () {}

                req.send(null);
            }
        }

        $(document).ready(function(){
            if ( Drupal.settings.bigbluebutton_view == 'joining' ){
                if (Drupal.settings.bigbluebutton_ismoderator == 'true' || Drupal.settings.bigbluebutton_waitformoderator == 'false'){
                    bigbluebutton_joinURL();
                } else {
                    $.jheartbeat.set({
                        url: Drupal.settings.bigbluebutton_pingURL,
                        delay: 2500
                        }, function() {
                            bigbluebutton_callback();
                    });
                }

            }
        });
