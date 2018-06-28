<?php

// server monitor scrip for remote monitoring and downtime reporting by
// telephone through twilio.


include( "serverMonitorSettings.php" );


global $urlToCheck, $expectedResponse, $downMessage;


$result = file_get_contents( $urlToCheck );


if( $result != $expectedResponse ) {
    echo "Server $urlToCheck is down.";
    
    sm_callAdmin( $downMessage );
    }
else {
    echo "Server $urlToCheck is up.";
    }




// makes a Twilio call to the admin with $inTextMessage as text-to-speech
function sm_callAdmin( $inTextMessage ) {

    
    global $twilioFromNumber, $twilioToNumber, $twilioAcountID,
        $twilioAuthToken;

    $fromParam = urlencode( $twilioFromNumber );
    $toParam = urlencode( $twilioToNumber );

    $encodedMessage = urlencode( $inTextMessage );


    // repeat 4 times
    $messageCopies =
        "Message%5B0%5D=$encodedMessage".
        "&".
        "Message%5B1%5D=$encodedMessage".
        "&".
        "Message%5B2%5D=$encodedMessage".
        "&".
        "Message%5B3%5D=$encodedMessage";
    

    $twimletURL = "http://twimlets.com/message?$messageCopies";
    
    $urlParam = urlencode( $twimletURL );
    
    
    global $curlPath;

    $curlCallString =
    "$curlPath -X POST ".
    "'https://api.twilio.com/2010-04-01/Accounts/$twilioAcountID/Calls.json' ".
    "-d 'To=$toParam'  ".
    "-d 'From=$fromParam' ".
    "-d ".
    "'Url=$urlParam' ".
    "-u $twilioAcountID:$twilioAuthToken";

    exec( $curlCallString );

    return;
    }

?>
