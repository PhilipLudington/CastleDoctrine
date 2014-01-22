#include "FetchRobberyReplayPage.h"
#include "ticketHash.h"
#include "message.h"

#include "serialWebRequests.h"

#include "nameProcessing.h"


#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"

#include "minorGems/util/stringUtils.h"


extern Font *mainFont;


extern char *serverURL;

extern int userID;



FetchRobberyReplayPage::FetchRobberyReplayPage() 
        : mWebRequest( -1 ),
          mRecordReady( false ),
          mMenuButton( mainFont, 4, -4, translate( "returnMenu" ) ),
          mReturnToMenu( false ) {

    // clearRecord will do the rest of the NULL-ing
    mLogRecord.robberName = NULL;
    clearRecord();

    addComponent( &mMenuButton );
    mMenuButton.addActionListener( this );
    }


        
FetchRobberyReplayPage::~FetchRobberyReplayPage() {
    if( mWebRequest != -1 ) {
        clearWebRequestSerial( mWebRequest );
        }
    clearRecord();
    }


void FetchRobberyReplayPage::clearRecord() {
    if( mLogRecord.robberName != NULL ) {
        delete [] mLogRecord.robberName;
        delete [] mLogRecord.victimName;
        delete [] mLogRecord.houseMap;
        delete [] mLogRecord.backpackContents;
        delete [] mLogRecord.moveList;   
        
        delete [] mLogRecord.wifeName;
        delete [] mLogRecord.sonName;
        delete [] mLogRecord.daughterName;
        }

    mLogRecord.robberName = NULL;
    mLogRecord.victimName = NULL;
    mLogRecord.houseMap = NULL;
    mLogRecord.backpackContents = NULL;
    mLogRecord.moveList = NULL;

    mLogRecord.wifeName = NULL;
    mLogRecord.sonName = NULL;
    mLogRecord.daughterName = NULL;
    
    mRecordReady = false;
    }



void FetchRobberyReplayPage::setLogID( int inID ) {
    mLogID = inID;
    }



char FetchRobberyReplayPage::getRecordReady() {
    return mRecordReady;
    }



char FetchRobberyReplayPage::getReturnToMenu() {
    return mReturnToMenu;
    }



RobberyLog FetchRobberyReplayPage::getLogRecord() {
    return mLogRecord;
    }



void FetchRobberyReplayPage::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == &mMenuButton ) {
        mReturnToMenu = true;
        }
    }







void FetchRobberyReplayPage::step() {
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
                    mStatusMessageKey = "robberyLogNotFound";
                    mMenuButton.setVisible( true );
                    }
                else {
                    // got a log!
                    
                    SimpleVector<char *> *tokens =
                        tokenizeString( result );
                    
                    if( tokens->size() != 12 ||
                        strcmp( *( tokens->getElement( 11 ) ), "OK" ) != 0 ) {

                        mStatusError = true;
                        mStatusMessageKey = "err_badServerResponse";
                        mMenuButton.setVisible( true );
                    
                        for( int i=0; i<tokens->size(); i++ ) {
                            delete [] *( tokens->getElement( i ) );
                            }
                        }
                    else {    
                        mLogRecord.robberName = 
                            nameParse( *( tokens->getElement( 0 ) ) );
                        mLogRecord.victimName = 
                            nameParse( *( tokens->getElement( 1 ) ) );
                        mLogRecord.houseMap = *( tokens->getElement( 2 ) );
                        mLogRecord.backpackContents = 
                            *( tokens->getElement( 3 ) );
                        mLogRecord.moveList = *( tokens->getElement( 4 ) );

                        char *lootToken = *( tokens->getElement( 5 ) );
                        
                        if( lootToken[0] == 'b' ) {
                            mLogRecord.isBounty = true;
                            
                            sscanf( lootToken,
                                    "b%d", &( mLogRecord.lootValue ) );
                            }
                        else {
                            mLogRecord.isBounty = false;
                            
                            sscanf( lootToken,
                                    "%d", &( mLogRecord.lootValue ) );
                            }
                        
                        sscanf( *( tokens->getElement( 6 ) ),
                                "%d", &( mLogRecord.wifeMoney ) );
                        sscanf( *( tokens->getElement( 7 ) ),
                                "%d", &( mLogRecord.musicSeed ) );
                        
                        mLogRecord.wifeName = *( tokens->getElement( 8 ) );
                        mLogRecord.sonName = *( tokens->getElement( 9 ) );
                        mLogRecord.daughterName = 
                            *( tokens->getElement( 10 ) );

                        delete [] *( tokens->getElement( 5 ) );
                        delete [] *( tokens->getElement( 6 ) );
                        delete [] *( tokens->getElement( 7 ) );
                        delete [] *( tokens->getElement( 11 ) );

                        mRecordReady = true;
                        }
                    
                    delete tokens;
                    }
                        
                        
                delete [] result;
                }
                break;
            }
        }
    }


        
void FetchRobberyReplayPage::makeActive( char inFresh ) {
    if( !inFresh ) {
        return;
        }

    clearRecord();
    
    char *ticketHash = getTicketHash();

    char *fullRequestURL = autoSprintf( 
        "%s?action=get_robbery_log&user_id=%d&log_id=%d"
        "&%s",
        serverURL, userID, mLogID, ticketHash );
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

