<?php

/**
 * Check to see if the meeting is active.
 *
 * Authors:
 *      Fred Dixon (ffdixon [at] blindsidenetworks [dt] org)
 *
 * @package   mod_bigbluebutton
 * @copyright 2010 Blindside Networks
 * @license   http://www.gnu.org/copyleft/gpl.html GNU GPL v2 or later
 */
include_once( "bbb_api/bbb_api.php" );
include_once( "bigbluebutton.admin.inc" );

$url_val = variable_get('bigbluebutton_server_url', '');
$salt_val = variable_get('bigbluebutton_server_salt', '');

$meetingID = $_GET['name'];

$salt = trim($SALT);
$url = trim(trim($URL),'/').'/';

echo BigBlueButton::getMeetingXML( $name, $url_val, $salt_val );

?>