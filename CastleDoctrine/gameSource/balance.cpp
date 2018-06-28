#include "balance.h"
#include "message.h"

#include "minorGems/game/doublePair.h"
#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"

#include "minorGems/util/stringUtils.h"


extern Font *mainFont;
extern Font *numbersFontFixed;


void drawBalance( int inLoot, int inChangeCost ) {
    doublePair labelPos = { 9, 2 };
    
    setDrawColor( 1, 1, 1, 1 );

    mainFont->drawString( translate( "editBalance" ), 
                          labelPos, alignRight );

    labelPos.y = 1.25;
    labelPos.x -= 0.125;
    char *balanceMessage = autoSprintf( "$%d", inLoot );

    numbersFontFixed->drawString( balanceMessage, 
                                  labelPos, alignRight );
    
    delete [] balanceMessage;



    if( inChangeCost >  0 ) {
        
        char costRed = ( inChangeCost > inLoot );

        
        labelPos.y = 0.5;
        
        char *costMessage = autoSprintf( "-$%d", inChangeCost );
        
        if( costRed ) {
            setDrawColor( 1, 0, 0, 1 );
            }
        else {
            // else green
            setDrawColor( 0.06666, 0.68627451, 0.454901961, 1 );
            }
        

        numbersFontFixed->drawString( costMessage, 
                                      labelPos, alignRight );

        delete [] costMessage;
        }

    }

