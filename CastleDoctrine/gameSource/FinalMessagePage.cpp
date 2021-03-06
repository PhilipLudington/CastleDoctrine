#include "FinalMessagePage.h"

#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"

#include "minorGems/util/stringUtils.h"

#include "message.h"


extern Font *mainFont;



FinalMessagePage::FinalMessagePage()
        : mKey( "" ), mSubMessage( NULL ) {

    }



FinalMessagePage::~FinalMessagePage() {
    setSubMessage( NULL );
    }




void FinalMessagePage::setMessageKey( const char *inKey ) {
    mKey = inKey;
    }



void FinalMessagePage::setSubMessage( const char *inSubMessage ) {
    if( mSubMessage != NULL ) {
        delete [] mSubMessage;
        mSubMessage = NULL;
        }
    if( inSubMessage != NULL ) {
        mSubMessage = stringDuplicate( inSubMessage );
        }
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

    if( mSubMessage != NULL ) {    
        labelPos.y -= 4;
        
        drawMessage( mSubMessage, labelPos, false );
        }
    
    }


