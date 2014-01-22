#include "FinalMessagePage.h"

#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"

#include "minorGems/util/stringUtils.h"

#include "message.h"


extern Font *mainFont;



FinalMessagePage::FinalMessagePage()
        : mKey( "" ) {

    }



void FinalMessagePage::setMessageKey( const char *inKey ) {
    mKey = inKey;
    }




void FinalMessagePage::makeActive( char inFresh ) {
    if( !inFresh ) {
        return;
        }
    
    setWaiting( false );
    }


void FinalMessagePage::draw( doublePair inViewCenter, 
                              double inViewSize ) {

    doublePair labelPos = { 0, 1 };

    
    drawMessage( translate( mKey ), labelPos, false ); 
    }


