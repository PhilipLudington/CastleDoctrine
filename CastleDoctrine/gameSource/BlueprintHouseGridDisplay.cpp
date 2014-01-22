#include "BlueprintHouseGridDisplay.h"



BlueprintHouseGridDisplay::BlueprintHouseGridDisplay( double inX, 
                                                        double inY )
        : HouseGridDisplay( inX, inY ) {

    mStepsBetweenHeldKeyRepeat = 4;
    mAllowKeyRepeatAcceleration = true;
    }


        
void BlueprintHouseGridDisplay::setHouseMap( const char *inHouseMap ) {
    HouseGridDisplay::setHouseMap( inHouseMap );
    
    // switch all to blueprint state
    // even stuck ones (don't represent burned-down walls, etc., in blueprint)
    resetToggledStates( 100, true );
    
    // robber invisible
    mRobberState = 100;
    }



void BlueprintHouseGridDisplay::specialKeyDown( int inKeyCode ) {
    // hijack key hold-down behavior from HouseGridDisplay
    if( mSpecialKeysHeldSteps[ inKeyCode ] == 0 ) {
        // not already down

        mSpecialKeysHeldSteps[ inKeyCode ] = 1;
        }

    // clear hold status of all other special keys
    for( int i=0; i<MG_KEY_LAST_CODE+1; i++ ) {
        if( i != inKeyCode ) {
            mSpecialKeysHeldSteps[i] = 0;
            }
        }
    

    int newOffsetX = mSubMapOffsetX;
    int newOffsetY = mSubMapOffsetY;


    if( inKeyCode == MG_KEY_LEFT ) {
        if( newOffsetX > 0 ) {
            newOffsetX --;
            }
        }
    else if( inKeyCode == MG_KEY_RIGHT ) {
        if( newOffsetX + HOUSE_D < mFullMapD ) {
            newOffsetX ++;
            }
        }
    else if( inKeyCode == MG_KEY_DOWN ) {
        if( newOffsetY > 0 ) {
            newOffsetY --;
            }
        }
    else if( inKeyCode == MG_KEY_UP ) {
        if( newOffsetY + HOUSE_D < mFullMapD ) {
            newOffsetY ++;
            }
        }

    if( newOffsetX != mSubMapOffsetX ||
        newOffsetY != mSubMapOffsetY ) {
        
        setVisibleOffset( newOffsetX, newOffsetY );

        fireActionPerformed( this );
        }
    
    
    }
