#!/usr/bin/perl -w 


my $wgetPath = "/usr/bin/wget"; 

my $numArgs = $#ARGV + 1;

if( $numArgs != 1 ) {
    usage();
    }

open( LIST_FILE, $ARGV[0] ) or usage();

print "opening $ARGV[0] for reading\n";

while( <LIST_FILE> ) {
    chomp;
    print "LINE: $_\n";
    (my $email, my $name) = split( /,\W*/ );
    $name =~ s/ /\+/g;

    print "  email = ($email), name = ($name)\n";



	my $url2 = "http://localhost/jcr13/castleServer/server.php?action=check_user&email=$email";

	print "  Checking user to create house:\n";
    print "  url = $url2\n";

	$result = `$wgetPath "$url2" -q -O -`;

    print "  result = $result\n";

    }



sub usage {
    print "Usage:\n";
    print "  bulk_housesOnly.pl list_file_name\n";
    print "Example:\n";
    print "  bulk_housesOnly.pl list.txt\n";
    print "List file must have one person per line in the following format:\n";
    print "  bob\@test.com, Bob Babbs\n";
    die;
    }
