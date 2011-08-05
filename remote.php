<?php //php definitions begin here
include_once( "bbb_api/bbb_api.php" );
require_once( "config.php" );

?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
		<title>Terminal Remote with JQuery</title>
		<link rel="stylesheet" href="style.css" type="text/css" />
		<script type="text/javascript" language="javascript" src="jquery/jquery1.6.2.js"></script>
		<script type="text/javascript" language="javascript" src="jquery/jquery.dataTables.js"></script>
		<script type="text/javascript" language="javascript" src="jquery/jquery.sha1.js"></script>
		<script type="text/javascript" language="javascript" src="jquery/heartbeat.js"></script>
		<script type="text/javascript" charset="utf-8">

<?php //Build the <head>

$meetingID = $_GET['name'];

if ( $meetingID != NULL && $meetingID != '' ){
	$logoutURL = "http://".$_SERVER['HTTP_HOST'].$_SERVER['REQUEST_URI'];
 	
	
	if ( ($realMeetingID = isMeetingCreated( $meetingID, $URL, $SALT )) == NULL ){
		$realMeetingID = $meetingID."--".time();
		$value = BigBlueButton::createMeeting($realMeetingID, $realMeetingID, $attendeePW, $moderatorPW, $welcome, $logoutURL, $SALT, $URL );
	}
	
	if ( BigBlueButton::isMeetingRunning( $realMeetingID, $URL, $SALT ) )
		$value = "Is running";
	else
		$value = "Is not running";
		
	$joinURL = BigBlueButton::joinURL( $realMeetingID, $meetingID, $attendeePW, $SALT, $URL );
?>

		htmlContent = "<?php echo $meetingID; ?> Terminal Remote ready for web conferencing.";
<?php	
} else {
?>	
		htmlContent = "Error: Terminal name has not been defined";
<?php
}	
?>
		//JQuery code STARTS
		$(document).ready(function() {
			$('#dynamic').html( htmlContent );
            
			$.jheartbeat.set({
              url: "<?php echo 'test.php?name='.urlencode($meetingID).'&logouturl='.$logoutURL; ?>",
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
              window.location = "<?php echo $joinURL; ?>";
           } else {
              //window.location.reload(true);
           }
        }
		</script>

	</head>
	<body>
      <div id="content">
        <center>
          <div id="dynamic"></div>
          <br /><br />
          <img src="polling.gif">
          <br /><br />
          Waiting for Practitioner/Clinician to initiate the call.
        </center>
      </div>
	</body>
</html>

<?php

function isMeetingCreated( $meetingID, $URL, $SALT ) {

	$xml = bbb_wrap_simplexml_load_file( BigBlueButton::getMeetingsURL( $URL, $SALT ) );
	if( $xml && $xml->returncode == 'SUCCESS' )
		foreach ($xml->meetings->meeting as $meeting){
			$meetingName = explode("--", $meeting->meetingID);
			if ( $meetingName[0] == $meetingID ){
				if ( $meeting->hasBeenForciblyEnded == 'false' )
					return $meeting->meetingID;
			}
		}
	return NULL;

}

?>