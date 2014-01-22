#!/usr/bin/perl -w 


my $wgetPath = "/usr/bin/wget"; 

my $numArgs = $#ARGV + 1;

if( $numArgs != 1 && $numArgs != 3 ) {
    usage();
    }


my $pngFilePath = $ARGV[0];

if( ! -e $pngFilePath ) {
	print "File \"$pngFilePath\" does not exist.\n\n";
	exit();
}


# find max ID number used so far in gallery
$maxGalleryID = 0;

$result = `find ./gameElements/gallery -name "info.txt"`;

@lines = split( /^/, $result );

foreach $line (@lines) {
	chomp $line;
	
	$fileContents = readFileValue( $line );

	@fileParts = split( /\s+/, $fileContents );

	$id = $fileParts[0];

	if( $id > $maxGalleryID ) {
		$maxGalleryID = $id;
	}
}

$newID = $maxGalleryID + 1;

print "Adding painting with ID $newID\n";


$title = "";
$author = "";

if( $numArgs != 3 ) {

	print "Enter painting title:  ";

	$title = <STDIN>;

	chomp( $title );
	
	print "Enter author (optional):  ";
	
	$author = <STDIN>;

	chomp( $author );
}
else {
	$title = $ARGV[1];
	$author = $ARGV[2];
}


$fullTitleText = $title;
$folderName = $title;

if( $author ne "" ) {
	$fullTitleText = "$title --- by $author";
	$folderName = "$author __ $title";
}

print "Full title text is \"$fullTitleText\"\n";


# no spaces in folder name
$folderName =~ s/\s+/_/g;

# no punctuation in folder name, other than _
$folderName =~ s/\W//g;


print "Folder name \"$folderName\"\n";

$path = "./gameElements/gallery/$folderName";

mkdir( $path );

open( INFO_FILE, ">$path/info.txt" ) or die;

print INFO_FILE "$newID\n\"$fullTitleText\"";

close INFO_FILE;


use File::Basename;

my( $pngFileName, $directories, $suffix ) = fileparse( $pngFilePath );

$tgaFileName = $pngFileName;

$tgaFileName =~ s/.png/.tga/;

$tgaFilePath = "$path/$tgaFileName";

print "TGA file is in $tgaFilePath\n";

$result = `convert $pngFilePath -type TrueColor $tgaFilePath`;



sub usage {
    print "Usage:\n";
    print "  addPainting.pl pngFile [paintingTitle paintingAuthor]\n";
    print "User is prompted for title/author if none provided.\n";
    print "Examples:\n";
    print "  addPainting.pl test.png\n";
    print "  addPainting.pl test.png \"A Test Painting\" \"Joe Blow\"\n";
    die;
    }



##
# Reads file as a string.
#
# @param0 the name of the file.
#
# @return the file contents as a string.
#
# Example:
# my $value = readFileValue( "myFile.txt" );
##
sub readFileValue {
    my $fileName = $_[0];
    open( FILE, "$fileName" ) or die;
    flock( FILE, 1 ) or die;

    # read the entire file, set the <> separator to nothing
    local $/;

    my $value = <FILE>;
    close FILE;

    return $value;
}
