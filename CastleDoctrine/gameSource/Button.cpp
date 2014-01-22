#include "Button.h"
#include "minorGems/game/drawUtils.h"
#include "minorGems/game/gameGraphics.h"
#include "minorGems/util/stringUtils.h"

#include <math.h>


Button::Button( double inX, double inY,
                double inWide, double inHigh,
                double inPixelSize )
        : PageComponent( inX, inY ),
          mHover( false ), mDragOver( false ),
          mWide( inWide ), mHigh( inHigh ), mPixWidth( inPixelSize ),
          mMouseOverTip( NULL ) {
    }


Button::~Button() {
    if( mMouseOverTip != NULL ) {
        delete [] mMouseOverTip;
        }
    }



void Button::setMouseOverTip( const char *inTipMessage ) {
    if( mMouseOverTip != NULL ) {
        delete [] mMouseOverTip;
        }

    if( inTipMessage != NULL ) {
        mMouseOverTip = stringDuplicate( inTipMessage );
        }
    else {
        mMouseOverTip = NULL;
        }
    }




void Button::setVisible( char inIsVible ) {
    PageComponent::setVisible( inIsVible );
        
    if( ! mVisible ) {
        clearState();
        }
    }



double Button::getWidth() {
    return mWide;
    }



void Button::clearState() {
    mHover = false;
    mDragOver = false;
    }


        
void Button::step() {
    }
        

void Button::draw() {
    if( mHover && ! mDragOver ) {    
        setDrawColor( 0.75, 0.75, 0.75, 1 );
        }
    else if( mDragOver ) {
        setDrawColor( 0.25, 0.25, 0.25, 1 );
        }
    else {
        setDrawColor( 0.5, 0.5, 0.5, 1 );
        }
    
    drawBorder();
    

    if( mDragOver ) {
        setDrawColor( 0.1, 0.1, 0.1, 1 );
        }
    else {
        setDrawColor( 0.25, 0.25, 0.25, 1 );
        }
    
    double rectStartX = - mWide / 2 + mPixWidth;
    double rectStartY = - mHigh / 2 + mPixWidth;

    double rectEndX = mWide / 2 - mPixWidth;
    double rectEndY = mHigh / 2 - mPixWidth;
    
    drawRect( rectStartX, rectStartY,
              rectEndX, rectEndY );

    if( mDragOver ) {    
        setDrawColor( 0.828, 0.647, 0.212, 1 );
        }
    else if( mHover ) {
        setDrawColor( 0.886, 0.764, 0.475, 1 );
        }
    else {
        setDrawColor( 1, 1, 1, 1 );
        }
    
    drawContents();
    }



void Button::drawContents() {
    }


void Button::drawBorder() {
    drawRect( - mWide / 2, - mHigh / 2, 
              mWide / 2, mHigh / 2 );
    }



char Button::isInside( float inX, float inY ) {
    return fabs( inX ) < mWide / 2 &&
        fabs( inY ) < mHigh / 2;
    }



void Button::pointerMove( float inX, float inY ) {
    if( isInside( inX, inY ) ) {
        mHover = true;
        if( mMouseOverTip != NULL ) {
            setToolTip( mMouseOverTip );
            }
        }
    else {
        if( mHover ) {
            // just hovered out
            setToolTip( NULL );
            }
        mHover = false;
        }
    }


void Button::pointerDown( float inX, float inY ) {
    pointerDrag( inX, inY );
    }



void Button::pointerDrag( float inX, float inY ) {
    if( isInside( inX, inY ) ) {
        mDragOver = true;
        if( mMouseOverTip != NULL ) {
            setToolTip( mMouseOverTip );
            }
        }
    else {
        if( mDragOver ) {
            // just dragged out
            setToolTip( NULL );
            }
        mDragOver = false;
        }
    mHover = false;
    }
        


void Button::pointerUp( float inX, float inY ) {
    mDragOver = false;
    if( isInside( inX, inY ) ) {
        mHover = true;
        setToolTip( "" );
        fireActionPerformed( this );
        }
    }        

