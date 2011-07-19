<?php
/**
* @file
* Administration page callbacks for the bigbluebutton module.
*/

require_once('bbb_api/bbb_api.php');

/**
* BigBlueButton: General Settings.
*
*/
function bigbluebutton_admin_settings() {

	$form['bigbluebutton_server_url'] = array(
		'#type' => 'textfield',
		'#title' => t('URL of BigBlueButton server'),
		'#description' => t('Enter the URL to access the BigBlueButton server, eg. "http://example.com/bigbluebutton/"'),
		'#default_value' => variable_get('bigbluebutton_server_url', ''),
		'#size' => 45
		);

	$form['bigbluebutton_server_salt'] = array(
		'#type' => 'textfield',
		'#title' => t('Salt of BigBlueButton server'),
		'#description' => t('Enter the Salt of the BigBlueButton server. It can be found in /var/lib/tomcat6/webapps/bigbluebutton/WEB-INF/classes/bigbluebutton.properties'),
		'#default_value' => variable_get('bigbluebutton_server_salt', ''),
		'#size' => 45
		);

	$form['#submit'][] = 'bigbluebutton_admin_settings_submit';

	return system_settings_form($form);
}

/**
 * BigBlueButton: Meeting room form.
 */
function bigbluebutton_admin_meeting_room_form($form, &$form_state, $op = 'add', $meeting_room_id = NULL ) {

	// If this is a new meeting room, the function will return an empty $meeting_room.
	$meeting_room = bigbluebutton_getMeeting_room($meeting_room_id);

  	$form['meeting_room_name'] = array(
    	'#type' => 'textfield',
    	'#title' => t('Meeting room'),
    	'#maxlength' => 35,
    	'#default_value' => $meeting_room['meeting_room_name'],
    	'#description' => t("Example: 'Demo Meeting'."),
    	'#required' => TRUE,
  		);

  	$form['attendee_pass'] = array(
    	'#type' => 'textfield',
    	'#title' => t('Attendee password'),
    	'#maxlength' => 25,
    	'#default_value' => $meeting_room['attendee_pass'],
    	'#description' => t("Example: 'ap' . To specify the password regular attendees should use to access the room."),
    	'#required' => TRUE,
  		);

  	$form['moderator_pass'] = array(
    	'#type' => 'textfield',
    	'#title' => t('Moderator password'),
    	'#maxlength' => 25,
    	'#default_value' => $meeting_room['moderator_pass'],
    	'#description' => t("Example: 'mp' . To specify the password moderators should use to access the room. Moderators have access to the presentation module and features to control the meeting."),
    	'#required' => TRUE,
  		);

  	$form['wait_for_moderator'] = array(
    	'#type' => 'checkbox',
    	'#title' => t('Wait for moderator to start meeting.'),
    	'#default_value' => $meeting_room['wait_for_moderator'],
    	'#description'   => t('Checking this box it will be required a moderator in the meeting in order to allow attendees get into.'),
  		);

  	$form['actions'] = array('#type' => 'actions');
  	$form['actions']['submit'] = array(
    	'#type' => 'submit',
    	'#value' => t('Submit'),
  		);

  	return $form;

}

/**
 * Process the contact category edit page form submission.
 */
function bigbluebutton_admin_meeting_room_form_submit($form, &$form_state) {

	//debug($form_state, "form_state", $print_r = TRUE);
	//debug($form, "form", $print_r = TRUE);

	if ( $form_state['build_info']['args'][0] == 'add' ) {
  		db_insert('meeting_room')
			->fields(array(
      			'meeting_room_name' => $form_state['values']['meeting_room_name'],
      			'meeting_version' => time(),
      			'attendee_pass' => $form_state['values']['attendee_pass'],
      			'moderator_pass' => $form_state['values']['moderator_pass'],
      			'wait_for_moderator' => $form_state['values']['wait_for_moderator'],
    			))
    		->execute();
	} else {
    	db_update('meeting_room')
			->condition('mid', $form_state['build_info']['args'][1])
			->fields(array(
      			'meeting_room_name' => $form_state['values']['meeting_room_name'],
      			'attendee_pass' => $form_state['values']['attendee_pass'],
      			'moderator_pass' => $form_state['values']['moderator_pass'],
      			'wait_for_moderator' => $form_state['values']['wait_for_moderator'],
    			))
    		->execute();
	}
	
  	drupal_set_message(t('Meeting room %meeting_room_name has been saved.', array('%meeting_room_name' => $form_state['values']['meeting_room_name'])));
  	$form_state['redirect'] = 'admin/config/system/bigbluebutton/meeting_room';

}


/**
 * @file
 * Defines functions for administrative interface for meeting rooms
 *
 */

/**
 * Meeting rooms/list tab.
 */
function bigbluebutton_admin_meeting_room_list() {
	$url_val = variable_get('bigbluebutton_server_url', '');
	$salt_val = variable_get('bigbluebutton_server_salt', '');

	// Initialize table headers
  	$header = array(
    	t('Meeting room'),
    	t('Attendee password'),
    	t('Moderator password'),
    	t('Status'),
    	array('data' => t('Operations'), 'colspan' => 3),
  		);
  	// Initialize rows array
	$rows = array();

  	// Get all the meeting rooms from the database.
	$meeting_rooms = db_select('meeting_room', 'm')
    	->fields('m', array('mid', 'meeting_room_name', 'attendee_pass', 'moderator_pass', 'wait_for_moderator'))
    	->orderBy('meeting_room_name')
    	->execute()
    	->fetchAll();

  	// Loop through the meeting rooms and add them to the table.
  	foreach ($meeting_rooms as $meeting_room) {
	    //debug(BigBlueButton::isMeetingRunning((string) $meeting_room->meeting_room_name, $url_val, $salt_val), "status", $print_r = TRUE);
		
    	$rows[] = array(
      		check_plain($meeting_room->meeting_room_name),
      		check_plain($meeting_room->attendee_pass),
      		check_plain($meeting_room->moderator_pass),
      		//(BigBlueButton::isMeetingRunning((string) $meeting_room->meeting_room_name, $url_val, $salt_val) ? "Running" : "Not running"),
      		(string) BigBlueButton::isMeetingRunning((string) $meeting_room->meeting_room_name, $url_val, $salt_val),
      		l(t('Edit'), 'admin/config/system/bigbluebutton/meeting_room/edit/' . $meeting_room->mid),
      		l(t('Test'), 'admin/config/system/bigbluebutton/meeting_room/test/' . $meeting_room->mid),
      		l(t('Delete'), 'admin/config/system/bigbluebutton/meeting_room/delete/' . $meeting_room->mid),
    	);
  	}

  	if (!$rows) {
    	$rows[] = array(array(
      		'data' => t('No meeting rooms available.'),
      		'colspan' => 5,
    		));
  	}

  	$build['meeting_rooms_table'] = array(
    	'#theme' => 'table',
    	'#header' => $header,
    	'#rows' => $rows,
  	);
  
  	return $build;

}

/**
 * Form builder for deleting a Meeting room.
 *
 */
function bigbluebutton_admin_meeting_room_delete_form($form, &$form_state,  $meeting_room_id = NULL) {

    $meeting_room = bigbluebutton_getMeeting_room($form_state['build_info']['args'][0]);

	$form['meeting_room_name'] = array(
    	'#type' => 'value',
    	'#value' => $meeting_room['meeting_room_name'],
 		 );

  	return confirm_form(
    	$form,
    	t('Are you sure you want to delete %meeting_room?', array('%meeting_room_name' => $meeting_room['meeting_room_name'])),
    	'admin/config/system/bigbluebutton/meeting_room',
    	t('This action cannot be undone.'),
    	t('Delete'),
    	t('Cancel')
  	);
}

/**
 * Submit handler for the confirm delete a Meeting room.
 *
 * @see contact_category_delete_form()
 */
function bigbluebutton_admin_meeting_room_delete_form_submit($form, &$form_state) {

	//debug($form_state, "form_state", $print_r = TRUE);

  	db_delete('meeting_room')
		->condition('mid', $form_state['build_info']['args'][0])
    	->execute();

  	drupal_set_message(t('Meeting room %meeting_room_name has been deleted.', array('%meeting_room_name' => $form_state['values']['meeting_room_name'])));

  	$form_state['redirect'] = 'admin/config/system/bigbluebutton/meeting_room';

}

function bigbluebutton_admin_meeting_room_test($form, &$form_state) {
	
	//---------------------------------------------------JOIN-----------------------------------------------
    $meeting_room = bigbluebutton_getMeeting_room($form_state['build_info']['args'][0]);
		
	$meetingID = $meeting_room['meeting_room_name'];
	$moderatorPW = $meeting_room['moderator_pass'];
	$attendeePW = $meeting_room['attendee_pass'];
	$meetingVersion = $meeting_room['meeting_version'];
	
	$url_val = variable_get('bigbluebutton_server_url', '');
	$salt_val = variable_get('bigbluebutton_server_salt', '');
	
	$user_name = $GLOBALS['user']->name;
	$logoutURL = $GLOBALS['_SERVER']['HTTP_REFERER'];

	//Calls create meeting on the bigbluebutton server
	$response = BigBlueButton::createMeetingArray($user_name, $meetingID."[".$meetingVersion."]", "", $moderatorPW, $attendeePW, $salt_val, $url_val, $logoutURL );

	$createNew = false;
	//Analyzes the bigbluebutton server's response
	if(!$response){//If the server is unreachable, then prompts the user of the necessary action
  		drupal_set_message(t('Unable to join the meeting room %meeting_room_name. Please check the url of the bigbluebutton server AND check to see if the bigbluebutton server is running.', array('%meeting_room_name' => $meeting_room['meeting_room_name'])));

	} else if( $response['returncode'] == 'FAILED' ) { //The meeting was not created
		if($response['messageKey'] == 'idNotUnique'){
			$createNew = true;
		}
		else if($response['messageKey'] == 'checksumError'){
  			drupal_set_message(t('A checksum error occured. Make sure you entered the correct salt.'));
		}
		else {
  			drupal_set_message(t($response['message']));
		}
	} else {
		$bbb_joinURL = BigBlueButton::joinURL($meetingID."[".$meetingVersion."]", $user_name, $moderatorPW, $salt_val, $url_val, $logoutURL );
		header('Location: '.$bbb_joinURL );
	}
	return;
	//---------------------------------------------------END-------------------------------------------------
}

function bigbluebutton_getMeeting_room($mid = NULL) {

  	$meeting_room = array(
    	'meeting_room_name' => '',
    	'attendee_pass' => '',
    	'moderator_pass' => '',
    	'wait_for_moderator' => 0,
  		);
	
	if ( $mid != NULL ) {		
		$results = db_select('meeting_room', 'm')
			->condition('mid', $mid)
    		->fields('m', array('mid', 'meeting_room_name', 'meeting_version', 'attendee_pass', 'moderator_pass', 'wait_for_moderator'))
    		->orderBy('meeting_room_name')
			->execute()
			->fetchAll();
		if ( $results != NULL ) {
	  			$meeting_room = array(
    			//'mid' => $meeting_room_id,
    			'meeting_room_name' => $results[0]->meeting_room_name,
    			'meeting_version' => $results[0]->meeting_version,
    			'attendee_pass' => $results[0]->attendee_pass,
    			'moderator_pass' => $results[0]->moderator_pass,
    			'wait_for_moderator' => $results[0]->wait_for_moderator,
  				);
		}
	}

	return $meeting_room;
}

?>