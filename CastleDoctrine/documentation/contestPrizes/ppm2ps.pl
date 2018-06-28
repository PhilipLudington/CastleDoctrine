#!/usr/bin/perl



my $fileName = $ARGV[0];

open( FILE, $fileName );

$line = <FILE>;

chomp $line;

if( $line ne "P3" ) {
	print "Only plain ppm's supported\n";
	exit;
    }

$line = <FILE>;
chomp $line;

$width = -1;
$height = -1;

( $width, $height ) = split( / /, $line );

$pageWidth = 20 * 72;
$pageHeight = 20 * 72;

$squareWidth = $pageWidth / $width;
$squareHeight = $pageWidth / $height;


$whiteBorder = 4 * 72;

$pageWidthB = $pageWidth + $whiteBorder;
$pageHeightB = $pageHeight + $whiteBorder;


#print "image is $width x $height\n";

$maxColorVal = <FILE>;
chomp $maxColorVal;


#print square-drawing function
print 
"%!PS-Adobe-3.0 EPSF-3.0
%%Creator: ppm2ps.pl (by Jason Rohrer)
%%Title: PS version of $fileName
%%CreationDate: June 22, 2008
%%DocumentData: Clean7Bit
%%Origin: 0 0
%%BoundingBox: -$whiteBorder -$whiteBorder $pageWidthB $pageHeightB
%%LanguageLevel: 2
%%Pages: 1
%%Page: 1 1
%%EOF

% draws a colored square using r g b from the stack
/csquare {
        newpath
        0    0   moveto
        0    $squareHeight.1 lineto
        $squareWidth.1  $squareHeight.1 lineto
        $squareWidth.1  0   lineto
        closepath
        setrgbcolor
        fill
} def


% draws a black border along bottom and left edge of a square
% (for overlap testing)
/csquare_edge {
        newpath
        0    0   moveto
        $squareWidth.5  0   lineto
        $squareWidth.5    $squareHeight.5 lineto
        0 0 0 setrgbcolor
        4 setlinewidth
        stroke
} def


% draws a colored square using r g b x y from the stack
/dp {
        gsave
        translate
        csquare
        grestore
} def



% draws a black line using x y x2 y2 from the stack
/dline {
        gsave
        0 0 0 setrgbcolor
        2 setlinewidth
        moveto
        lineto
        stroke
        grestore
} def


%2 2 scale

";


# switch to reading in space-delim mode
$/ = " ";




for( $y=0; $y<$height; $y++ ) {
	for( $x=0; $x<$width; $x++ ) {
		
		my $r = <FILE>;
		$r = trim( $r );
		my $g = <FILE>;
		$g = trim( $g );
		my $b = <FILE>;
		$b = trim( $b );
		
		my $pageY = $height - 1 - $y;

		my $squareX = $x * $squareWidth;
		my $squareY = $pageY * $squareHeight;

		$r = $r / $maxColorVal;
		$g = $g / $maxColorVal;
		$b = $b / $maxColorVal;


		if( $x == 0 && $y == 0 ) {
			for( $i=4; $i>1; $i-- ) {
				printf( "%.3f %.3f %.3f %d %d dp\n", 
						$r, $g, $b, 
						$squareX - $i * $squareWidth, 
						$squareY + $squareHeight );
			}
		}

		if( $y == 0 ) {
			for( $i=4; $i>0; $i-- ) {
				if( $x == 0 ) {
					printf( "%.3f %.3f %.3f %d %d dp\n", 
							$r, $g, $b, 
							$squareX - $squareWidth, 
							$squareY + $i * $squareHeight );
				}

				printf( "%.3f %.3f %.3f %d %d dp\n", 
						$r, $g, $b, $squareX, $squareY + $i * $squareHeight );

				if( $x == $width - 1 ) {
					printf( "%.3f %.3f %.3f %d %d dp\n", 
							$r, $g, $b, 
							$squareX + $squareWidth, 
							$squareY + $i * $squareHeight );
				}
			}
		}

		if( $x == 0 ) {
			for( $i=4; $i>0; $i-- ) {

				printf( "%.3f %.3f %.3f %d %d dp\n", 
						$r, $g, $b, $squareX - $i * $squareWidth, $squareY );
				
				if( $y == $height - 1 ) {
					printf( "%.3f %.3f %.3f %d %d dp\n", 
							$r, $g, $b, 
							$squareX - $i * $squareWidth, 
							$squareY - $squareHeight );
				}
			}
		}




		
		printf( "%.3f %.3f %.3f %d %d dp\n", 
				$r, $g, $b, $squareX, $squareY );

		
		if( $y == $height - 1 ) {
			for( $i=1; $i<=4; $i++ ) {
				if( $x == 0 ) {
					printf( "%.3f %.3f %.3f %d %d dp\n", 
							$r, $g, $b, 
							$squareX - $squareWidth, 
							$squareY - $i * $squareHeight );
				}

				printf( "%.3f %.3f %.3f %d %d dp\n", 
						$r, $g, $b, $squareX, $squareY - $i * $squareHeight );
			}
		}



		if( $x == $width - 1 ) {
			for( $i=1; $i<=4; $i++ ) {
				if( $y == 0 ) {
					printf( "%.3f %.3f %.3f %d %d dp\n", 
							$r, $g, $b, 
							$squareX + $i * $squareWidth, 
							$squareY + $squareHeight );
				}
				printf( "%.3f %.3f %.3f %d %d dp\n", 
						$r, $g, $b, $squareX + $i * $squareWidth, $squareY );
				if( $y == $height - 1 ) {
					printf( "%.3f %.3f %.3f %d %d dp\n", 
							$r, $g, $b, 
							$squareX + $i * $squareWidth, 
							$squareY - $squareHeight );
				}
			}

			if( $y == $height - 1 && $x == $width - 1 ) {
				for( $i=2; $i<=4; $i++ ) {
					
					printf( "%.3f %.3f %.3f %d %d dp\n", 
							$r, $g, $b, 
							$squareX + $squareWidth, 
							$squareY - $i * $squareHeight );
				}
			}
		}

	}
}


# tickmarks

my $tickLength = 72;

printf( "%d 0  %d  0 dline\n",
		-$whiteBorder, -$whiteBorder + $tickLength );

printf( "%d 0  %d  0 dline\n",
		$pageWidthB, $pageWidthB - $tickLength );

printf( "%d $pageHeight  %d  $pageHeight dline\n",
		-$whiteBorder, -$whiteBorder + $tickLength );

printf( "%d $pageHeight  %d  $pageHeight dline\n",
		$pageWidthB, $pageWidthB - $tickLength );


printf( "0 %d 0  %d dline\n",
		-$whiteBorder, -$whiteBorder + $tickLength );

printf( "0 %d 0  %d dline\n",
		$pageHeightB, $pageHeightB - $tickLength );


printf( "$pageWidth %d $pageWidth  %d dline\n",
		-$whiteBorder, -$whiteBorder + $tickLength );

printf( "$pageWidth %d $pageWidth  %d dline\n",
		$pageHeightB, $pageHeightB - $tickLength );

print "\nshowpage\n";


# Perl trim function to remove whitespace from the start and end of the string
sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}




