#!/usr/bin/perl -w 


my $wgetPath = "/usr/bin/wget"; 

my $numArgs = $#ARGV + 1;

if( $numArgs != 1 ) {
    usage();
    }

my $ID = 3000;
my $price = 500;

my $numPaintings = $ARGV[0];

for( $i=0; $i<$numPaintings; $i++ ) {
	
	$title = "Dummy $i --- by No One";

	print "    array( $ID, $price, 1, \"$title\" ),\n";

	$dirName = "gameElements/gallery/dummy_$i"; 
	mkdir( $dirName );
	
	$infoName = "$dirName/info.txt";

	open( LIST_FILE, ">$infoName" );
	
	print LIST_FILE "$ID\n\"$title\"";
	
	close LIST_FILE;
	
	$imageName = "$dirName/dummy$i.tga";

	`convert -size 32x32 -background white -fill blue -stroke blue label:$i $imageName`;
	
	$ID++;
}


sub usage {
    print "Usage:\n";
    print "  makeDummyPaintings.pl numPaintings\n";
    print "Example:\n";
    print "  makeDummyPaintings.pl 15\n";
    die;
    }
