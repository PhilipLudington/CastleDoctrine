#include "FetchBlueprintPage.h"
#include "ticketHash.h"
#include "message.h"
#include "nameProcessing.h"
#include "LiveHousePage.h"

#include "serialWebRequests.h"


#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"

#include "minorGems/util/stringUtils.h"


extern Font *mainFont;


extern char *serverURL;

extern int userID;



FetchBlueprintPage::FetchBlueprintPage() 
        : mWebRequest( -1 ),
          mOwnerName( NULL ),
          mHouseMap( NULL ),
          mMenuButton( mainFont, 4, -4, translate( "returnMenu" ) ),
          mReturnToMenu( false ),
          mToRobCharacterName( NULL ) {

    addComponent( &mMenuButton );
    mMenuButton.addActionListener( this );
    }


void FetchBlueprintPage::clearDataMembers() {
    if( mOwnerName != NULL ) {
        delete [] mOwnerName;
        }
    mOwnerName = NULL;
    
    if( mHouseMap != NULL ) {
        delete [] mHouseMap;
        }
    mHouseMap = NULL;
    }


        
FetchBlueprintPage::~FetchBlueprintPage() {
    if( mWebRequest != -1 ) {
        clearWebRequestSerial( mWebRequest );
        }
    clearDataMembers();

    if( mToRobCharacterName != NULL ) {
        delete [] mToRobCharacterName;
        }
    mToRobCharacterName = NULL;
    }



void FetchBlueprintPage::setToRobUserID( int inID ) {
    mToRobUserID = inID;
    }


void FetchBlueprintPage::setToRobCharacterName( const char *inName ) {
    if( mToRobCharacterName != NULL ) {
        delete [] mToRobCharacterName;
        }
    mToRobCharacterName = stringDuplicate( inName );
    }



char FetchBlueprintPage::getReturnToMenu() {
    return mReturnToMenu;
    }




char *FetchBlueprintPage::getHouseMap() {
    if( mHouseMap == NULL ) {
        return NULL;
        }
    else {
        return stringDuplicate( mHouseMap );
        }
    }


char *FetchBlueprintPage::getOwnerName() {
    if( mOwnerName == NULL ) {
        return NULL;
        }
    else {
        return stringDuplicate( mOwnerName );
        }
    }



void FetchBlueprintPage::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == &mMenuButton ) {
        mReturnToMenu = true;
        }
    }


void FetchBlueprintPage::step() {
    if( mWebRequest != -1 ) {
            
        int stepResult = stepWebRequestSerial( mWebRequest );
        
        if( stepResult != 0 ) {
            setWaiting( false );
            }

        switch( stepResult ) {
            case 0:
                break;
            case -1:
                mStatusError = true;
                mStatusMessageKey = "err_webRequest";
                clearWebRequestSerial( mWebRequest );
                mWebRequest = -1;
                mMenuButton.setVisible( true );
                break;
            case 1: {
                char *result = getWebResultSerial( mWebRequest );
                clearWebRequestSerial( mWebRequest );
                mWebRequest = -1;
                     
                printf( "Web result = %s\n", result );
   
                if( strstr( result, "DENIED" ) != NULL ) {
                    mStatusError = true;
                    mStatusMessageKey = "blueprintNotFound";
                    mMenuButton.setVisible( true );
                    }
                else if( strstr( result, "RECLAIMED" ) != NULL ) {
                    mStatusError = true;
                    mStatusMessageKey = "houseReclaimed";
                    mMenuButton.setVisible( true );
                    }
                else {
                    // house checked out!
                    
                    SimpleVector<char *> *tokens =
                        tokenizeString( result );
                    
                    if( tokens->size() != 3 ||
                        strcmp( *( tokens->getElement( 2 ) ), "OK" ) != 0 ) {
                        mStatusError = true;
                        mStatusMessageKey = "err_badServerResponse";
                        mMenuButton.setVisible( true );
                    
                        for( int i=0; i<tokens->size(); i++ ) {
                            delete [] *( tokens->getElement( i ) );
                            }
                        }
                    else {
                        mOwnerName = nameParse( *( tokens->getElement( 0 ) ) );
                        mHouseMap = *( tokens->getElement( 1 ) );
                        
                        printf( "OwnerName = %s\n", mOwnerName );
                        printf( "HouseMap = %s\n", mHouseMap );
                        

                        delete [] *( tokens->getElement( 2 ) );
                        }
                    
                    delete tokens;
                    }
                        
                        
                delete [] result;
                }
                break;
            }
        }
    }


        
void FetchBlueprintPage::makeActive( char inFresh ) {
    if( !inFresh ) {
        return;
        }

    clearDataMembers();
        
    char *ticketHash = getTicketHash();

    char *fullRequestURL = autoSprintf( 
        "%s?action=get_blueprint&user_id=%d&to_rob_user_id=%d"
        "&to_rob_character_name=%s&%s",
        serverURL, userID, mToRobUserID, mToRobCharacterName, ticketHash );
    delete [] ticketHash;
    
    mWebRequest = startWebRequestSerial( "GET", 
                                   fullRequestURL, 
                                   NULL );

    delete [] fullRequestURL;
    
    mMenuButton.setVisible( false );

    mStatusError = false;
    mStatusMessageKey = NULL;

    mReturnToMenu = false;
    
    setWaiting( true );
    }

