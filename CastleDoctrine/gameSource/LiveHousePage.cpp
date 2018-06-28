#include "LiveHousePage.h"

#include "ticketHash.h"

#include "serialWebRequests.h"

#include "minorGems/game/game.h"

#include "minorGems/util/stringUtils.h"




extern char *serverURL;

extern int userID;


// static init
// set to 0 for now because we can't safely call game_time during init
int LiveHousePage::sLastPingTime = 0;


int LiveHousePage::sWebRequest = -1;


SimpleVector<const char *> LiveHousePage::sPendingTestRequests;



LiveHousePage::LiveHousePage()
        : mLastActionTime( game_time( NULL ) ),
          mCheckoutStale( false ),
          mOutOfTime( false ),
          mStartTestFailed( false ),
          mCurrentRequestForStartTest( false ) {
    
    // assume ping sent at startup, because no house checked out at startup,
    // and checking out a house automatically pings it
    
    if( sLastPingTime == 0 ) {
        // we're the first to set it

        // other LiveHousePage's will find it already set at their construction

        sLastPingTime = game_time( NULL );
        }
    }



LiveHousePage::~LiveHousePage() {
    if( sWebRequest != -1 ) {
        clearWebRequestSerial( sWebRequest );
        sWebRequest = -1;
        }
    }



void LiveHousePage::makeActive( char inFresh ) {
    if( !inFresh ) {
        return;
        }

    // this page becoming active is an action
    mLastActionTime = game_time( NULL );
    mCheckoutStale = false;
    mOutOfTime = false;
    mStartTestFailed = false;
    mCurrentRequestForStartTest = false;
    }



void LiveHousePage::actionHappened() {
    mLastActionTime = game_time( NULL );
    }


void LiveHousePage::startSelfTest() {
    sPendingTestRequests.push_back( "start_self_test" );
    }


void LiveHousePage::endSelfTest() {
    sPendingTestRequests.push_back( "end_self_test" );
    }



void LiveHousePage::step() {
    if( sWebRequest != -1 ) {
            
        int result = stepWebRequestSerial( sWebRequest );
          
        if( result != 0 ) {
            // send is over, not matter what response we get back
            setWaiting( false );
            
            // same response possibilies for all requests types here
            
            switch( result ) {
                case -1:
                    mCheckoutStale = true;
                    if( mCurrentRequestForStartTest ) {
                        mStartTestFailed = true;
                        }
                    printf( "Web request FAILED!\n" );
                    break;
                case 1: {
                    char *response = getWebResultSerial( sWebRequest );

                    printf( "Server response:  %s\n", response );
                    
                    // same OK result expected whether we
                    // have sent a ping or a self-test start/end
                    if( strstr( response, "OK" ) == NULL ) {
                        mCheckoutStale = true;
                        if( mCurrentRequestForStartTest ) {
                            mStartTestFailed = true;
                            }
                        
                        if( strstr( response, "OUT_OF_TIME" ) != NULL ) {
                            mOutOfTime = true;
                            }
                        }
                    else if( strstr( response, 
                                     "SERVER_GOING_DOWN" ) != NULL ) {
                        showShutdownPendingWarning();
                        }
                    

                    delete [] response;
                    }
                }

            clearWebRequestSerial( sWebRequest );
            sWebRequest = -1;
            }
        }
    else if( sPendingTestRequests.size() > 0 ) {

        const char *command = *( sPendingTestRequests.getElement( 0 ) );
        sPendingTestRequests.deleteElement( 0 );        

        if( strcmp( command, "start_self_test" ) == 0 ) {
            mCurrentRequestForStartTest = true;
            }
        else {
            mCurrentRequestForStartTest = false;
            }

        char *ticketHash = getTicketHash();
            
        char *fullRequestURL = autoSprintf( 
            "%s?action=%s&user_id=%d"
            "&%s",
            serverURL, command, userID, ticketHash );
        delete [] ticketHash;
        
        sWebRequest = startWebRequestSerial( "GET", 
                                       fullRequestURL, 
                                       NULL );

        // show warnings only for these behind-the-scenes
        // requests
        setWaiting( true, true );
        
        delete [] fullRequestURL;
        
        // counts as a ping
        sLastPingTime = game_time( NULL );
        }
    else if( ! mCheckoutStale ) {
        int currentTime = game_time( NULL );
        
        if( currentTime > sLastPingTime + 60 * 4 ) {
            // getting close to five minute timeout mark
            
            if( currentTime - mLastActionTime < 60 * 5 ) {
                // there's been activity in the last five minutes
                
                // send ping
                
                char *ticketHash = getTicketHash();
            
                char *fullRequestURL = autoSprintf( 
                    "%s?action=ping_house&user_id=%d"
                    "&%s",
                    serverURL, userID, ticketHash );
                delete [] ticketHash;
                
                sWebRequest = startWebRequestSerial( "GET", 
                                               fullRequestURL, 
                                               NULL );
                
                // show warnings only for these behind-the-scenes
                // requests
                setWaiting( true, true );

                mCurrentRequestForStartTest = false;
                
                delete [] fullRequestURL;
                
                sLastPingTime = currentTime;
                }
            }
        
        }
    
    }



char LiveHousePage::areRequestsPending() {
    if( sWebRequest != -1 ||
        sPendingTestRequests.size() > 0 ) {
        return true;
        }

    return false;
    }



