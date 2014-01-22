#include "EyedropperStatusDisplay.h"

#include <math.h>


#include "minorGems/game/game.h"
#include "minorGems/util/log/AppLog.h"


EyedropperStatusDisplay::EyedropperStatusDisplay( double inX, double inY,
                                                  double inDrawScale )
        : PageComponent( inX, inY ), mHover( false ),
          mOnSprite( NULL ), mOffSprite( NULL ),
          mDrawScale( inDrawScale ) {
            
    
    const char *tgaName = "eyedropperOn.tga";    

    Image *image = readTGAFile( tgaName );

    if( image != NULL ) {
        mWide = image->getWidth() * inDrawScale;
        mHigh = image->getHeight() * inDrawScale;
        
        mOnSprite = fillSprite( image );
        
        delete image;
        }
    else {
        AppLog::errorF( "Failed to read ON sprite for Eyedropper display: %s",
                        tgaName );
        }

    
    tgaName = "eyedropperOff.tga";
    image = readTGAFile( tgaName );

    if( image != NULL ) {
        
        mOffSprite = fillSprite( image );
        
        delete image;
        }
    else {
        AppLog::errorF( "Failed to read OFF sprite for Eyedropper display: %s",
                        tgaName );
        }
    }



        
EyedropperStatusDisplay::~EyedropperStatusDisplay() {
    
    if( mOnSprite != NULL ) {
        freeSprite( mOnSprite );
        }
    if( mOffSprite != NULL ) {
        freeSprite( mOffSprite );
        }
    }




char EyedropperStatusDisplay::isInside( float inX, float inY ) {
    return fabs( inX ) < mWide / 2 &&
        fabs( inY ) < mHigh / 2;
    }



void EyedropperStatusDisplay::pointerMove( float inX, float inY ) {
    if( isInside( inX, inY ) ) {
        mHover = true;
        setToolTip( translate( "eyedropperTip" ) );
        }
    else {
        if( mHover ) {
            // just hovered out
            setToolTip( NULL );
            }
        mHover = false;
        }
    }



void EyedropperStatusDisplay::pointerDown( float inX, float inY ) {
    pointerMove( inX, inY );
    }



void EyedropperStatusDisplay::pointerDrag( float inX, float inY ) {
    pointerMove( inX, inY );
    }



void EyedropperStatusDisplay::pointerUp( float inX, float inY ) {
    pointerMove( inX, inY );
    }



void EyedropperStatusDisplay::draw() {
    
    doublePair center = { 0, 0 };
    setDrawColor( 1, 1, 1, 1 );


    if( isCommandKeyDown() ) {    
        if( mOnSprite != NULL ) {
            drawSprite( mOnSprite, center, mDrawScale );
            }
        }
    else {    
        if( mOffSprite != NULL ) {
            drawSprite( mOffSprite, center, mDrawScale );
            }
        }
    
        
    }
