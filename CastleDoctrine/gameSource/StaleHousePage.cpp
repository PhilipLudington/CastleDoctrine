#include "StaleHousePage.h"

#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"

#include "minorGems/util/stringUtils.h"


extern Font *mainFont;


StaleHousePage::StaleHousePage( char inDead ) 
        : mDoneButton( mainFont, 4, -4, 
                       translate( 
                           ( inDead ? "startOver" : "doneStale" ) ) ),
          mDone( false ),
          mDead( inDead ),
          mOutOfTime( false ) {    

    addComponent( &mDoneButton );
    mDoneButton.addActionListener( this );
    }



void StaleHousePage::setOutOfTime( char inOutOfTime ) {
    mOutOfTime = inOutOfTime;
    }



char StaleHousePage::getDone() {
    return mDone;
    }



void StaleHousePage::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == &mDoneButton ) {
        mDone = true;
        }
    }



void StaleHousePage::makeActive( char inFresh ) {
    if( !inFresh ) {
        return;
        }
    
    mDone = false;
    
    mStatusError = true;

    if( mDead ) {
        
        if( mOutOfTime ) {
            mStatusMessageKey = "houseStaleOutOfTime";
            }
        else {
            mStatusMessageKey = "houseStaleDead";
            }
        }
    else {
        mStatusMessageKey = "houseStale";
        }
    }


