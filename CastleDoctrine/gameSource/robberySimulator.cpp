#include "ReplayRobHouseGridDisplay.h"
#include "inventory.h"

#include "minorGems/util/stringUtils.h"

ReplayRobHouseGridDisplay *replayCheckerDisplay;



void initSimulator() {
    replayCheckerDisplay = new ReplayRobHouseGridDisplay( 0, 0 );
    }

void freeSimulator() {
    delete replayCheckerDisplay;
    }




char *simulateRobbery( const char *inRobberyData ) {

    SimpleVector<char *> *tokens = tokenizeString( inRobberyData );
    
    if( tokens->size() != 5 ) {
        
        for( int i=0; i<tokens->size(); i++ ) {
            delete [] *( tokens->getElement( i ) );
            }
        
        return stringDuplicate( "FAILED" );
        }

    
    /*
      === Call:
simulate_robbery
houseMap
backpack_contents
moveList
wife_money
    */
    
    replayCheckerDisplay->setHouseMap( *( tokens->getElement(1) ) );

    SimpleVector<QuantityRecord> backpackSlots;    
    fromString( *( tokens->getElement(2) ), &backpackSlots );
    
    
    replayCheckerDisplay->setMoveList( *( tokens->getElement(3) ) );

    int wifeMoney = 0;
    sscanf( *( tokens->getElement(4) ), "%d", &wifeMoney );
    
    replayCheckerDisplay->setWifeMoney( wifeMoney );
    
    replayCheckerDisplay->setWifeName( "SimWife" );
    replayCheckerDisplay->setSonName( "SimSon" );
    replayCheckerDisplay->setDaughterName( "SimDaughter" );



                          
    for( int i=0; i<tokens->size(); i++ ) {
        delete [] *( tokens->getElement( i ) );
        }
    delete tokens;
    

    replayCheckerDisplay->playAtFullSpeed();
    

    char moveListIncorrect = false;
    
    while( !replayCheckerDisplay->getDead() && 
           replayCheckerDisplay->getSuccess() == 0 &&
           ! replayCheckerDisplay->getMoveListExhausted() ) {
        
        replayCheckerDisplay->step();

        if( replayCheckerDisplay->didForbiddenMoveHappen() ) {
            moveListIncorrect = true;
            break;
            }

        if( replayCheckerDisplay->getToolJustUsed() ) {
            
            int pickedID = replayCheckerDisplay->getToolIDJustPicked();
            
            if( pickedID != -1 ) {
                char toolAllowed = subtractFromQuantity( &backpackSlots,
                                                         pickedID );

                if( !toolAllowed ) {
                    moveListIncorrect = true;
                    break;
                    }
                }
            }
        }
    

    if( !replayCheckerDisplay->getDead() && 
        replayCheckerDisplay->getSuccess() == 0 ) {
        // move list ran out or was incorrect before robbery ended properly!
        
        return stringDuplicate( "FAILED" );
        }
    
    if( replayCheckerDisplay->getDead() && 
        replayCheckerDisplay->getDeathSourceID() == -1 ) {
        // dead, but not killed by anything
        // suicide.

        replayCheckerDisplay->processFamilyAndMobilesAtEnd();
        }

    
    /*
Returns:
success
wife_killed
wife_robbed
family_killed_count
end_backpack_contents
end_house_map
    */
    
    char *response;

    if( !moveListIncorrect ) {
        
        char *endBackpack = toString( &backpackSlots );


        char *endHouseMap = replayCheckerDisplay->getHouseMap();
        
        response = 
            autoSprintf( "%d\n"
                         "%d\n"
                         "%d\n"
                         "%d\n"
                         "%d\n"
                         "%s\n"
                         "%s",
                         replayCheckerDisplay->getSuccess(),
                         replayCheckerDisplay->getWifeKilledRobber(),
                         replayCheckerDisplay->getWifeKilled(),
                         replayCheckerDisplay->getWifeRobbed(),
                         replayCheckerDisplay->getFamilyKilledCount(),
                         endBackpack,
                         endHouseMap );


        delete [] endBackpack;
        delete [] endHouseMap;
        }
    else {
        response = stringDuplicate( "FAILED" );
        }

    return response;
    }

