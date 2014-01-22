#include "RobCheckoutHousePage.h"
#include "ticketHash.h"
#include "message.h"
#include "nameProcessing.h"
#include "LiveHousePage.h"

#include "serialWebRequests.h"

#include "sha1Encryption.h"

#include "secureString.h"

#include "mapEncryptionKey.h"


#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"

#include "minorGems/util/stringUtils.h"
#include "minorGems/crypto/hashes/sha1.h"


extern Font *mainFont;


extern char *serverURL;

extern int userID;



RobCheckoutHousePage::RobCheckoutHousePage() 
        : mWebRequest( -1 ),
          mMapEncryptionKey( NULL ),
          mWifeName( NULL ),
          mSonName( NULL ),
          mDaughterName( NULL ),
          mOwnerName( NULL ),
          mHouseMap( NULL ),
          mBackpackContents( NULL ),
          mGalleryContents( NULL ),
          mMenuButton( mainFont, 4, -4, translate( "returnMenu" ) ),
          mReturnToMenu( false ),
          mToRobCharacterName( NULL ) {

    addComponent( &mMenuButton );
    mMenuButton.addActionListener( this );
    }


void RobCheckoutHousePage::clearDataMembers() {
    if( mWifeName != NULL ) {
        delete [] mWifeName;
        }
    mWifeName = NULL;
    
    if( mSonName != NULL ) {
        delete [] mSonName;
        }
    mSonName = NULL;
    
    if( mDaughterName != NULL ) {
        delete [] mDaughterName;
        }
    mDaughterName = NULL;
    
    if( mOwnerName != NULL ) {
        delete [] mOwnerName;
        }
    mOwnerName = NULL;
    
    if( mHouseMap != NULL ) {
        clearString( mHouseMap );
        }
    mHouseMap = NULL;
    
    if( mBackpackContents != NULL ) {
        delete [] mBackpackContents;
        }
    mBackpackContents = NULL;
    
    if( mGalleryContents != NULL ) {
        delete [] mGalleryContents;
        }
    mGalleryContents = NULL;
    
    }


        
RobCheckoutHousePage::~RobCheckoutHousePage() {
    if( mWebRequest != -1 ) {
        clearWebRequestSerial( mWebRequest );
        }
    
    if( mMapEncryptionKey != NULL ) {
        delete [] mMapEncryptionKey;
        }

    clearDataMembers();

    if( mToRobCharacterName != NULL ) {
        delete [] mToRobCharacterName;
        }
    mToRobCharacterName = NULL;
    }



void RobCheckoutHousePage::setToRobHomeID( int inID ) {
    mToRobHomeID = inID;
    }


void RobCheckoutHousePage::setToRobCharacterName( const char *inName ) {
    if( mToRobCharacterName != NULL ) {
        delete [] mToRobCharacterName;
        }
    mToRobCharacterName = stringDuplicate( inName );
    }



char RobCheckoutHousePage::getReturnToMenu() {
    return mReturnToMenu;
    }




char *RobCheckoutHousePage::getWifeName() {
    if( mWifeName == NULL ) {
        return NULL;
        }
    else {
        return stringDuplicate( mWifeName );
        }
    }


char *RobCheckoutHousePage::getSonName() {
    if( mSonName == NULL ) {
        return NULL;
        }
    else {
        return stringDuplicate( mSonName );
        }
    }


char *RobCheckoutHousePage::getDaughterName() {
    if( mDaughterName == NULL ) {
        return NULL;
        }
    else {
        return stringDuplicate( mDaughterName );
        }
    }



char *RobCheckoutHousePage::getHouseMap() {
    if( mHouseMap == NULL ) {
        return NULL;
        }
    else {
        char *returnValue = stringDuplicate( mHouseMap );
        clearString( mHouseMap );
        mHouseMap = NULL;
        
        return returnValue;
        }
    }


char *RobCheckoutHousePage::getOwnerName() {
    if( mOwnerName == NULL ) {
        return NULL;
        }
    else {
        return stringDuplicate( mOwnerName );
        }
    }



char *RobCheckoutHousePage::getBackpackContents() {
    if( mBackpackContents == NULL ) {
        return NULL;
        }
    else {
        return stringDuplicate( mBackpackContents );
        }
    }



char *RobCheckoutHousePage::getGalleryContents() {
    if( mGalleryContents == NULL ) {
        return NULL;
        }
    else {
        return stringDuplicate( mGalleryContents );
        }
    }


int RobCheckoutHousePage::getWifeMoney() {
    return mWifeMoney;
    }


int RobCheckoutHousePage::getMusicSeed() {
    return mMusicSeed;
    }


int RobCheckoutHousePage::getMaxSeconds() {
    return mMaxSeconds;
    }




void RobCheckoutHousePage::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == &mMenuButton ) {
        mReturnToMenu = true;
        }
    }


void RobCheckoutHousePage::step() {
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
                blockQuitting( false );
                break;
            case 1: {
                char *result = getWebResultSerial( mWebRequest );
                clearWebRequestSerial( mWebRequest );
                mWebRequest = -1;
                     
                printf( "Web result = %s\n", result );
   
                if( strstr( result, "DENIED" ) != NULL ) {
                    mStatusError = true;
                    mStatusMessageKey = "houseBeingRobbedOrEdited";
                    mMenuButton.setVisible( true );
                    blockQuitting( false );
                    }
                else if( strstr( result, "RECLAIMED" ) != NULL ) {
                    mStatusError = true;
                    mStatusMessageKey = "houseReclaimed";
                    mMenuButton.setVisible( true );
                    blockQuitting( false );
                    }
                else if( strstr( result, "CHILLING" ) != NULL ) {
                    mStatusError = true;
                    mStatusMessageKey = "houseChilling";
                    mMenuButton.setVisible( true );
                    blockQuitting( false );
                    }
                else {
                    // house checked out!
                    
                    SimpleVector<char *> *tokens =
                        tokenizeString( result );
                    
                    if( tokens->size() != 11 ||
                        strcmp( *( tokens->getElement( 10 ) ), "OK" ) != 0 ) {
                        mStatusError = true;
                        mStatusMessageKey = "err_badServerResponse";
                        mMenuButton.setVisible( true );
                    
                        for( int i=0; i<tokens->size(); i++ ) {
                            delete [] *( tokens->getElement( i ) );
                            }
                        }
                    else {
                        mOwnerName = nameParse( *( tokens->getElement( 0 ) ) );

                        mHouseMap = 
                            sha1Decrypt( mMapEncryptionKey,
                                         *( tokens->getElement( 1 ) ) );
                        delete [] *( tokens->getElement( 1 ) );
                        

                        mBackpackContents = *( tokens->getElement( 2 ) );
                        mGalleryContents = *( tokens->getElement( 3 ) );

                        mWifeMoney = 0;
                        sscanf( *( tokens->getElement( 4 ) ),
                                "%d", &mWifeMoney );
                        mMusicSeed = 0;
                        sscanf( *( tokens->getElement( 5 ) ),
                                "%d", &mMusicSeed );
                        
                        mWifeName = *( tokens->getElement( 6 ) );
                        mSonName = *( tokens->getElement( 7 ) );
                        mDaughterName = *( tokens->getElement( 8 ) );
                        
                        mMaxSeconds = 0;
                        sscanf( *( tokens->getElement( 9 ) ),
                                "%d", &mMaxSeconds );

                        printf( "OwnerName = %s\n", mOwnerName );
                        // printf( "HouseMap = %s\n", mHouseMap );
                        printf( "Backpack = %s\n", mBackpackContents );
                        printf( "Gallery = %s\n", mGalleryContents );
                        printf( "WifeMoney = %d\n", mWifeMoney );
                        printf( "MusicSeed = %d\n", mMusicSeed );
                        printf( "Wife = %s\n", mWifeName );
                        printf( "Son = %s\n", mSonName );
                        printf( "Daughter = %s\n", mDaughterName );
                        printf( "MaxSeconds = %d\n", mMaxSeconds );


                        delete [] *( tokens->getElement( 4 ) );
                        delete [] *( tokens->getElement( 5 ) );
                        delete [] *( tokens->getElement( 9 ) );
                        delete [] *( tokens->getElement( 10 ) );

                        // reset ping time, because house check-out
                        // counts as a ping
                        LiveHousePage::sLastPingTime = game_time( NULL );
                        }
                    
                    delete tokens;
                    }
                        
                        
                delete [] result;
                }
                break;
            }
        }
    }





        
void RobCheckoutHousePage::makeActive( char inFresh ) {
    if( !inFresh ) {
        return;
        }

    clearDataMembers();
        
    char *ticketHash = getTicketHash();




    if( mMapEncryptionKey != NULL ) {
        delete [] mMapEncryptionKey;
        }

    mMapEncryptionKey = getMapEncryptionKey();

    

    char *fullRequestURL = autoSprintf( 
        "%s?action=start_rob_house&user_id=%d&to_rob_home_id=%d"
        "&to_rob_character_name=%s&map_encryption_key=%s&%s",
        serverURL, userID, mToRobHomeID, mToRobCharacterName, 
        mMapEncryptionKey, ticketHash );
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

