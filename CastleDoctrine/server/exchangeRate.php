<?php





include( "exchangeRateSettings.php" );





global $databaseServer, $databaseUsername, $databasePassword, $databaseName;



mysql_connect( $databaseServer, $databaseUsername, $databasePassword )
or die( "Could not connect to database server: " .
        mysql_error() );
    
mysql_select_db( $databaseName )
or die( "Could not select $databaseName database: " .
        mysql_error() );


global $startDate, $endDate;
global $tableNamePrefix;


$query = "SELECT MAX( max_total_house_value ) ".
"FROM $tableNamePrefix"."server_stats ".
"WHERE stat_date >= '$startDate' AND stat_date <= '$endDate';";


$result = mysql_query( $query );


$max_total_house_value = mysql_result( $result, 0, 0 );


global $totalUSDValue;

$exchangeRate = floor( $max_total_house_value / $totalUSDValue );



echo "<font size=5>\$$exchangeRate in game = \$1 USD</font>";


$sampleHouseValues = array( 2000, 10000, 50000, 100000 );

echo "<br><br>At the end of the contest:<br>";


echo "<TABLE BORDER=1 cellpadding=5>";
echo
"<tr><td><b>If your cash + tools are worth:</b></td><td><b>You will win:</b></td>";


foreach( $sampleHouseValues as $sample ) {
    $usdValue = floor( $sample / $exchangeRate );
    echo "<tr><td align=right>\$$sample</td>";
    echo "<td align=right>\$$usdValue USD</td></tr>";
    }
echo "</table>";


?>