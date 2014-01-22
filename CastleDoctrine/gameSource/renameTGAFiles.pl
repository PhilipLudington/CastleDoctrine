#!/usr/bin/perl -w 


my $numArgs = $#ARGV + 1;

if( $numArgs != 2 ) {
    usage();
    }

$result = `find . -name "*$ARGV[0]*.tga"`;

@lines = split( /^/, $result );

foreach $line (@lines) {
	chomp $line;
	
	print "found $line\n";

	$oldLine = $line;
	

	$line =~ s/$ARGV[0]/$ARGV[1]/g;

	print "|--> changed to $line\n\n";

	`mv $oldLine $line`;
}


sub usage {
    print "Usage:\n";
    print "  renameTGAFiles.pl search_name replacement_name\n";
    print "Example:\n";
    print "  renameTGAFiles.pl woodWall metalWall\n";
    die;
    }
