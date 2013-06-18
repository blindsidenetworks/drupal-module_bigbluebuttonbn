<?php
/*
Plugin Name: BigBlueButtonBN
Plugin URI: http://blindsidenetworks.com/integration
Author: Blindside Networks
Author URI: http://blindsidenetworks.com/
Description: BigBlueButton is an open source web conferencing system. This module integrates BigBlueButton into Drupal allowing website administrators to create and manage meetings rooms that can be used by visitors and registred users. For more information on setting up your own BigBlueButton server or for using an external hosting provider visit http://bigbluebutton.org/support

Copyright 2011 Blindside Networks

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Versions:
7.x-1.0.x-dev	--  	Initial version written by Jesus Federico
(email : jesus [a t ] b l i n d si de n  et w o rks DOT com)
 
*/

/**
 * @file
* Administration page callbacks for the bigbluebutton module.
*/

require_once('bbb_api/bbb_api.php');

/**
 * BigBlueButton: General Settings.
 *
 */
function bigbluebuttonbn_admin_settings() {

    $form['bigbluebuttonbn_server_url'] = array(
            '#type' => 'textfield',
            '#title' => t('URL of BigBlueButton server'),
            '#description' => t('Enter the URL to access the BigBlueButton server, eg. "http://example.com/bigbluebutton/"'),
            '#default_value' => variable_get('bigbluebuttonbn_server_url', 'http://test-install.blindsidenetworks.com/bigbluebutton/'),
            '#size' => 45
    );

    $form['bigbluebuttonbn_server_salt'] = array(
            '#type' => 'textfield',
            '#title' => t('Salt of BigBlueButton server'),
            '#description' => t('Enter the Salt of the BigBlueButton server. It can be found in /var/lib/tomcat6/webapps/bigbluebutton/WEB-INF/classes/bigbluebutton.properties'),
            '#default_value' => variable_get('bigbluebuttonbn_server_salt', '8cd8ef52e8e101574e400365b55e11a6'),
            '#size' => 45
    );

    $form['#submit'][] = 'bigbluebuttonbn_admin_settings_submit';

    return system_settings_form($form);
}

/**
 * BigBlueButton: Meeting room form.
 */
function bigbluebuttonbn_admin_meeting_form($form, &$form_state, $op = 'add', $meeting_id = NULL ) {

    // If this is a new meeting room, the function will return an empty $meeting.
    $meeting = bigbluebuttonbn_getMeeting($meeting_id);

    $form['meeting_name'] = array(
            '#type' => 'textfield',
            '#title' => t('Meeting room'),
            '#maxlength' => 35,
            '#default_value' => $meeting['meeting_name'],
            '#description' => t("Example: 'Demo Meeting'."),
            '#required' => TRUE,
  		);

    $form['attendee_pass'] = array(
            '#type' => 'textfield',
            '#title' => t('Viewer password'),
            '#maxlength' => 25,
            '#default_value' => $meeting['attendee_pass'],
            '#description' => t("Example: 'ap' . To specify the password regular attendees should use to access the room."),
            '#required' => TRUE,
  		);

    $form['moderator_pass'] = array(
            '#type' => 'textfield',
            '#title' => t('Moderator password'),
            '#maxlength' => 25,
            '#default_value' => $meeting['moderator_pass'],
            '#description' => t("Example: 'mp' . To specify the password moderators should use to access the room. Moderators have access to the presentation module and features to control the meeting."),
            '#required' => TRUE,
  		);
    
    $form['wait_for_moderator'] = array(
            '#type' => 'checkbox',
            '#title' => t('Wait for moderator to start meeting.'),
            '#default_value' => $meeting['wait_for_moderator'],
            '#description'   => t('Checking this box it will be required a moderator in the meeting in order to allow attendees get into.'),
  		);

    /*
    $form['newwindow'] = array(
            '#type' => 'checkbox',
            '#title' => t('Open meeting in a new window.'),
            '#default_value' => $meeting['newwindow'],
            '#description'   => t('Checking this box it will open a new window to join meetings.'),
  		);
    */
    
    $form['record'] = array(
            '#type' => 'checkbox',
            '#title' => t('Playback recording for meeting on/off.'),
            '#default_value' => $meeting['record'],
            '#description'   => t('Checking this box it will activate the playback recording for the current meeting.'),
  		);
    
    $form['duration'] = array(
            '#type' => 'textfield',
            '#title' => t('Duration'),
            '#maxlength' => 3,
            '#default_value' => $meeting['duration'],
            '#description' => t("Set the duration in minutes expected for the meeting to last. After this time all the users will be kicked out."),
    );

    $form['description'] = array(
            '#type' => 'textfield',
            '#title' => t('Description for the playback recording.'),
            '#maxlength' => 64,
            '#default_value' => $meeting['description'],
            '#description'   => t('Description for the play back recording related to the current meeting.'),
  		);

    $form['welcome'] = array(
            '#type' => 'textarea',
            '#title' => t('Welcome message.'),
            '#default_value' => $meeting['welcome'],
            '#description'   => t('Message to be displayed in the chat window as welcoming once the user logs in.'),
  		);

    $form['actions'] = array('#type' => 'actions');
    $form['actions']['submit'] = array(
            '#type' => 'submit',
            '#value' => t('Submit'),
  		);
    $form['actions']['cancel'] = array(
            //To be shown as a default cancel link in drupal 
            '#type' => 'markup',
            '#markup' => l(t('Cancel'), 'admin/config/content/bigbluebuttonbn/meeting'),
            //To be shown as a submit button
            //'#type' => 'button',
            //'#executes_submit_callback' => true,
            '#value' => t('Cancel'),
    );
    
    return $form;

}

/**
 * Process the contact category edit page form submission.
 */
function bigbluebuttonbn_admin_meeting_form_submit($form, &$form_state) {
    
    if( isset($form_state['input']['op']) && $form_state['input']['op'] == 'Submit' ){
        if ( $form_state['build_info']['args'][0] == 'add' ) {
            db_insert('bigbluebuttonbn')
            ->fields(array(
                    'meeting_id' => bigbluebuttonbn_rand_string( 16 ),
                    'meeting_name' => $form_state['values']['meeting_name'],
                    'attendee_pass' => $form_state['values']['attendee_pass'],
                    'moderator_pass' => $form_state['values']['moderator_pass'],
                    'wait_for_moderator' => $form_state['values']['wait_for_moderator'],
                    'newwindow' => isset($form_state['values']['newwindow'])? $form_state['values']['newwindow']: 0,
                    'record' => $form_state['values']['record'],
                    'duration' => $form_state['values']['duration'],
                    'description' => $form_state['values']['description'],
                    'welcome' => $form_state['values']['welcome'],
            ))
            ->execute();
        } else {
            db_update('bigbluebuttonbn')
            ->condition('mid', $form_state['build_info']['args'][1])
            ->fields(array(
                    'meeting_name' => $form_state['values']['meeting_name'],
                    'attendee_pass' => $form_state['values']['attendee_pass'],
                    'moderator_pass' => $form_state['values']['moderator_pass'],
                    'wait_for_moderator' => $form_state['values']['wait_for_moderator'],
                    'newwindow' => isset($form_state['values']['newwindow'])? $form_state['values']['newwindow']: 0,
                    'record' => $form_state['values']['record'],
                    'duration' => $form_state['values']['duration'],
                    'description' => $form_state['values']['description'],
                    'welcome' => $form_state['values']['welcome'],
            ))
            ->execute();
        }

        drupal_set_message(t('Meeting room %meeting_name has been saved.', array('%meeting_name' => $form_state['values']['meeting_name'])));
        
    }
    
    $form_state['redirect'] = 'admin/config/content/bigbluebuttonbn/meeting';

}


/**
 * @file
 * Defines functions for administrative interface for meeting rooms
 *
 */

/**
 * Meeting rooms/list tab.
 */
function bigbluebuttonbn_admin_meeting_list() {
    $URL = variable_get('bigbluebuttonbn_server_url', '');
    $SALT = variable_get('bigbluebuttonbn_server_salt', '');

    // Initialize table headers
    $header = array(
            t('Meeting room'),
            t('Viewer password'),
            t('Moderator password'),
            t('Status'),
            array('data' => t('Operations'), 'colspan' => 4),
  		);
    // Initialize rows array
    $rows = array();

    // Get all the meeting rooms from the database.
    $meetings_ = db_select('bigbluebuttonbn', 'm')
    ->fields('m', array('mid', 'meeting_id', 'meeting_name', 'attendee_pass', 'moderator_pass', 'wait_for_moderator'))
    ->orderBy('meeting_name')
    ->execute()
    ->fetchAll();

    // Loop through the meeting rooms and add them to the table.
    foreach ($meetings_ as $meeting) {
        $rows[] = array(
                check_plain($meeting->meeting_name),
                check_plain($meeting->attendee_pass),
                check_plain($meeting->moderator_pass),

                (BigBlueButtonBN::isMeetingRunning((string) $meeting->meeting_id, $URL, $SALT) ? "Running" : "Not running"),
                l(t('Edit'), 'admin/config/content/bigbluebuttonbn/meeting/edit/' . $meeting->mid),
                l(t('Delete'), 'admin/config/content/bigbluebuttonbn/meeting/delete/' . $meeting->mid),
                l(t('Join'), 'admin/config/content/bigbluebuttonbn/meeting/join/' . $meeting->mid),
                BigBlueButtonBN::isMeetingRunning((string) $meeting->meeting_id, $URL, $SALT) ? l(t('End'), 'admin/config/content/bigbluebuttonbn/meeting/end/' . $meeting->mid): ''
        );
    }

    if (!$rows) {
        $rows[] = array(array(
                'data' => t('No meeting rooms available.'),
                'colspan' => 5,
        ));
    }

    $build['meetings__table'] = array(
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
function bigbluebuttonbn_admin_meeting_delete_form($form, &$form_state,  $meeting_id = NULL) {

    $meeting = bigbluebuttonbn_getMeeting($form_state['build_info']['args'][0]);

    $form['meeting_name'] = array(
            '#type' => 'value',
            '#value' => $meeting['meeting_name'],
 		 );

    return confirm_form(
            $form,
            t('Are you sure you want to delete %meeting_name?', array('%meeting_name' => $meeting['meeting_name'])),
            'admin/config/content/bigbluebuttonbn/meeting',
            t('This action cannot be undone.'),
            t('Delete'),
            t('Cancel')
    );
}

/**
 * Submit handler for confirmation of Meeting deletion.
 *
 * @see contact_category_delete_form()
 */
function bigbluebuttonbn_admin_meeting_delete_form_submit($form, &$form_state) {

    db_delete('bigbluebuttonbn')
    ->condition('mid', $form_state['build_info']['args'][0])
    ->execute();

    drupal_set_message(t('Meeting room %meeting_name has been deleted.', array('%meeting_name' => $form_state['values']['meeting_name'])));

    $form_state['redirect'] = 'admin/config/content/bigbluebuttonbn/meeting';

}

/**
 * Form builder for ending a Meeting that is running.
 *
 */
function bigbluebuttonbn_admin_meeting_end_form($form, &$form_state,  $meeting_id = NULL) {

    $meeting = bigbluebuttonbn_getMeeting($form_state['build_info']['args'][0]);
    
    $form['meeting_id'] = array(
            '#type' => 'value',
            '#value' => $meeting['meeting_id'],
    );
    $form['meeting_name'] = array(
            '#type' => 'value',
            '#value' => $meeting['meeting_name'],
    );
    $form['moderator_pass'] = array(
            '#type' => 'value',
            '#value' => $meeting['moderator_pass'],
    );
    
    return confirm_form(
            $form,
            t('Are you sure you want to end %meeting_name?', array('%meeting_name' => $meeting['meeting_name'])),
            'admin/config/content/bigbluebuttonbn/meeting',
            t('This action cannot be undone.'),
            t('End'),
            t('Cancel')
    );
}

/**
 * Submit handler for confirmation of Meeting ending.
 *
 * @see contact_category_delete_form()
 */
function bigbluebuttonbn_admin_meeting_end_form_submit($form, &$form_state) {

    $URL = variable_get('bigbluebuttonbn_server_url', '');
    $SALT = variable_get('bigbluebuttonbn_server_salt', '');
    
    $meetingID = $form_state['values']['meeting_id'];
    $moderatorPW = $form_state['values']['moderator_pass'];
    $response = BigBlueButtonBN::endMeeting($meetingID, $moderatorPW, $URL, $SALT );

    //Analyzes the bigbluebutton server's response
    if(!$response) {//If the server is unreachable, then prompts the user of the necessary action
        drupal_set_message(t('Unable to terminate the meeting. Please check the url of the bigbluebutton server AND check to see if the bigbluebutton server is running.'));
    
    } else if( $response['returncode'] == 'SUCCESS' ) { //The meeting was terminated
        drupal_set_message(t('Meeting session %meeting_name has been ended. All users have been kicked out.', array('%meeting_name' => $form_state['values']['meeting_name'])));
    
    } else { //If the meeting was unable to be termindated
        if($response['messageKey'] == 'checksumError'){
            drupal_set_message(t('A checksum error occured. Make sure you entered the correct salt.'));
        
        } else {
            drupal_set_message(t($response['message']));

        }

    }

    $form_state['redirect'] = 'admin/config/content/bigbluebuttonbn/meeting';

}

function bigbluebuttonbn_admin_meeting_join($form, &$form_state) {
    
    //debug($form_state, "form_state", $print_r = TRUE);
    
    $meeting = bigbluebuttonbn_getMeeting($form_state['build_info']['args'][0]);

    $user = $GLOBALS['user'];
    $meeting['user_name'] = $user->name;
    $meeting['user_password'] = $meeting['moderator_pass'];


    if ( ($bbb_joinURL = bigbluebuttonbn_joinMeeting($meeting)) != NULL )
        header('Location: '.$bbb_joinURL );
}


/**
 * Recording list tab.
 */
function bigbluebuttonbn_admin_recording_list() {
    return bigbluebuttonbn_recording_list();
    
}


/**
 * Form builder for deleting a Recording from the BigBlueButton server.
 *
 */
function bigbluebuttonbn_admin_recording_delete_form($form, &$form_state,  $meeting_id = NULL) {

    $meeting = bigbluebuttonbn_getMeeting($form_state['build_info']['args'][0]);

    $form['meeting_name'] = array(
            '#type' => 'value',
            '#value' => $meeting['meeting_name'],
 		 );

    return confirm_form(
            $form,
            t('Are you sure you want to delete %meeting_name?', array('%meeting_name' => $meeting['meeting_name'])),
            'admin/config/content/bigbluebuttonbn/recording',
            t('This action cannot be undone.'),
            t('Delete'),
            t('Cancel')
    );
}

/**
 * Submit handler for confirmation of Recording deletion.
 *
 * @see contact_category_delete_form()
 */
function bigbluebuttonbn_admin_recording_delete_form_submit($form, &$form_state) {
    //debug($form_state, "form_state", $print_r = TRUE);
    
    $URL = variable_get('bigbluebuttonbn_server_url', '');
    $SALT = variable_get('bigbluebuttonbn_server_salt', '');
    
    $recordID = $form_state['build_info']['args'][0];
    BigBluebuttonBN::deleteRecordings($recordID, $URL, $SALT);

    drupal_set_message(t('Recording selected has been deleted.', array('%meeting_name' => $form_state['values']['meeting_name'])));

    $form_state['redirect'] = 'admin/config/content/bigbluebuttonbn/recording';

}


/**
 * Handler for publish Recording.
 *
 */
function bigbluebuttonbn_admin_recording_show($form, &$form_state) {
    //debug($form_state, "form_state", $print_r = TRUE);

    $URL = variable_get('bigbluebuttonbn_server_url', '');
    $SALT = variable_get('bigbluebuttonbn_server_salt', '');

    $recordID = $form_state['build_info']['args'][0];
    BigBluebuttonBN::publishRecordings($recordID, 'true', $URL, $SALT);

    drupal_set_message(t('Recording selected has been published.'));

    //header('Location: ./?q=admin/config/content/bigbluebuttonbn/recording' );
    drupal_goto('admin/config/content/bigbluebuttonbn/recording');
}

/**
 * Handler for unpublish Recording.
 *
 */
function bigbluebuttonbn_admin_recording_hide($form, &$form_state) {
    //debug($form_state, "form_state", $print_r = TRUE);

    $URL = variable_get('bigbluebuttonbn_server_url', '');
    $SALT = variable_get('bigbluebuttonbn_server_salt', '');

    $recordID = $form_state['build_info']['args'][0];
    BigBluebuttonBN::publishRecordings($recordID, 'false', $URL, $SALT);
    
    drupal_set_message(t('Recording selected has been unpublished.'));

    //header('Location: ./?q=admin/config/content/bigbluebuttonbn/recording' );
    drupal_goto('admin/config/content/bigbluebuttonbn/recording');
}


function bigbluebuttonbn_joinMeeting($meeting){
    //---------------------------------------------------JOIN-----------------------------------------------
    $bbb_joinURL = NULL;

    $meetingID = $meeting['meeting_id'];
    $meetingName = $meeting['meeting_name'];
    $moderatorPW = $meeting['moderator_pass'];
    $attendeePW = $meeting['attendee_pass'];
    $welcome = $meeting['welcome'];
    $welcome = (( !isset($meeting['welcome']) || $meeting['welcome'] == '')? "Welcome to [$meetingName] Meeting Room.": $meeting['welcome'])."<br>";
    $record = $meeting['record'] == 1? 'true': 'false';
    $welcome .= $record == 'true'? "<br><b>This meeting is being recorded.</b><br>": '';
    $duration = $meeting['duration'];
    $welcome .= $duration > 0? "<br><b>The maximum duration for this meeting is ".$duration." minutes.</b><br>": '';
    $description = $meeting['description'];
    
    $URL = variable_get('bigbluebuttonbn_server_url', '');
    $SALT = variable_get('bigbluebuttonbn_server_salt', '');
    $logoutURL = $GLOBALS['base_url'];

    $user_name = $meeting['user_name'];
    $user_password = $meeting['user_password'];
    
    //Validate the password
    if ( $user_password != $moderatorPW && $user_password != $attendeePW ) {
        drupal_set_message(t("The password typed doesn't match neither the attendee password nor the moderator password registered."));

    } else {
        //Calls create meeting on the bigbluebutton server
        if ( !BigBlueButtonBN::isMeetingCreated( $meetingID, $URL, $SALT ) ){
            $response = BigBlueButtonBN::createMeetingArray($meetingName, $meetingID, $attendeePW, $moderatorPW,  $URL, $SALT, $logoutURL, $welcome, $record, $duration, 0, array("meta_description" => $description, "meta_email" => '') );

            //Analyzes the bigbluebutton server's response
            if(!$response){//If the server is unreachable, then prompts the user of the necessary action
                drupal_set_message(t('Unable to join the meeting room %meeting_name. Please check the url of the bigbluebutton server AND check to see if the bigbluebutton server is running.', array('%meeting_name' => $meeting['meeting_name'])));
                return;
            } else if( $response['returncode'] == 'FAILED' ) { //The meeting was not created
                if($response['messageKey'] == 'checksumError'){
                    drupal_set_message(t('A checksum error occured. Make sure you entered the correct salt.'));
                } else {
                    drupal_set_message(t($response['message']));
                }
                return;
            } else {
                if( !isset($response['messageKey']) || $response['messageKey'] == '' ){
                    // The meeting was just created, insert the create event to the log
                    db_insert('bigbluebuttonbn_logs')
                    ->fields(array(
                            'meeting_id' => $meetingID,
                            'record' => $record? 1: 0,
                            'timestamp' => time(),
                            'event' => 'Create',
                    ))
                    ->execute();
                }
                
                $bbb_joinURL = BigBlueButtonBN::joinURL($meetingID, $user_name, $user_password, $URL, $SALT, $logoutURL );
            }
        } else  {
            $bbb_joinURL = BigBlueButtonBN::joinURL($meetingID, $user_name, $user_password, $URL, $SALT, $logoutURL );
        }
    }
    //---------------------------------------------------END-------------------------------------------------
    return $bbb_joinURL;
}


function bigbluebuttonbn_getMeeting($mid = NULL) {

    $meeting = array(
            'mid' => '',
            'meeting_id' => '',
            'meeting_name' => '',
            'attendee_pass' => '',
            'moderator_pass' => '',
            'wait_for_moderator' => 0,
            'newwindow' => 0,
            'record' => 0,
            'duration' => 0,
            'description' => '',
            'welcome' => '',
    );
    
    if ( $mid != NULL ) {
        $results = db_select('bigbluebuttonbn', 'm')
        ->fields('m', array('mid', 'meeting_id', 'meeting_name', 'attendee_pass', 'moderator_pass', 'wait_for_moderator', 'newwindow', 'record', 'duration', 'description', 'welcome'))
        ->condition('mid', $mid, '=')
        ->orderBy('meeting_name')
        ->execute()
        ->fetchAll();

        if ( $results != NULL ) {
            $meeting = array(
                    'mid' => $results[0]->mid,
                    'meeting_id' => $results[0]->meeting_id,
                    'meeting_name' => $results[0]->meeting_name,
                    'attendee_pass' => $results[0]->attendee_pass,
                    'moderator_pass' => $results[0]->moderator_pass,
                    'wait_for_moderator' => $results[0]->wait_for_moderator,
                    'newwindow' => $results[0]->newwindow,
                    'record' => $results[0]->record,
                    'duration' => $results[0]->duration,
                    'description' => $results[0]->description,
                    'welcome' => $results[0]->welcome,
            );
        }
    }

    return $meeting;

}


function bigbluebuttonbn_getAllMeetings() {

    $results = db_select('bigbluebuttonbn', 'm')
    ->fields('m', array('mid', 'meeting_id', 'meeting_name', 'attendee_pass', 'moderator_pass', 'wait_for_moderator', 'newwindow', 'record', 'duration', 'description', 'welcome'))
    ->orderBy('meeting_name')
    ->execute()
    ->fetchAll();

    foreach( $results as $rowset ) {
        $meetings_[$rowset->mid] = array(
                'mid' => $rowset->mid,
                'meeting_id' => $rowset->meeting_id,
                'meeting_name' => $rowset->meeting_name,
                'attendee_pass' => $rowset->attendee_pass,
                'moderator_pass' => $rowset->moderator_pass,
                'wait_for_moderator' => $rowset->wait_for_moderator,
                'newwindow' => $rowset->newwindow,
                'record' => $rowset->record,
                'duration' => $rowset->duration,
                'description' => $rowset->description,
                'welcome' => $rowset->welcome,
        );
    }

    return $meetings_;
}

function bigbluebuttonbn_getAllRecordings() {

    $URL = variable_get('bigbluebuttonbn_server_url', '');
    $SALT = variable_get('bigbluebuttonbn_server_salt', '');
    
    $results = db_select('bigbluebuttonbn_logs', 'm')
        ->fields('m', array('id', 'meeting_id', 'record', 'timestamp', 'event'))
        ->distinct()
        ->condition('event', 'Create', '=')
        ->condition('record', 1, '=')
        ->orderBy('timestamp')
        ->execute()
        ->fetchAll();

    $meetingIds = '';
    foreach( $results as $rowset ) {
        if ( strlen($meetingIds) > 0 ) $meetingIds .= ',';
        $meetingIds .= $rowset->meeting_id;
    }
    if( $meetingIds != '')
        $recordings = BigBlueButtonBN::getRecordingsArray($meetingIds, $URL, $SALT);
    else
        $recordings = array();

    return $recordings;
}

/***
 * Any other bigbluebuttonbn functions go here.  Each of them must have a name that
* starts with bigbluebuttonbn_
* Remember (see note in first lines) that, if this section grows, it's HIGHLY
* recommended to move all funcions below to a new "localib.php" file.
**
*  Function taken from http://www.php.net/manual/en/function.mt-rand.php
*  modified by Sebastian Schneider
*  credits go to www.mrnaz.com
*/
function bigbluebuttonbn_rand_string($len, $chars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789')
{
    $string = '';
    for ($i = 0; $i < $len; $i++)
    {
        $pos = rand(0, strlen($chars)-1);
        $string .= $chars{$pos};
    }
    return (sha1($string));
}

?>