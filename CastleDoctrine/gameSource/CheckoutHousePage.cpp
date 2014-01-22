#include "CheckoutHousePage.h"
#include "ticketHash.h"
#include "message.h"
#include "LiveHousePage.h"

#include "serialWebRequests.h"



#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"

#include "minorGems/util/stringUtils.h"


extern Font *mainFont;


extern char *serverURL;

extern int userID;



CheckoutHousePage::CheckoutHousePage() 
        : mWebRequest( -1 ),
          mWifeName( NULL ),
          mSonName( NULL ),
          mDaughterName( NULL ),
          mPaymentCount( 0 ),
          mYouPaidTotal( 0 ),
          mWifePaidTotal( 0 ),
          mNumberOfTapes( 0 ),
          mHouseMap( NULL ),
          mVaultContents( NULL ),
          mBackpackContents( NULL ),
          mGalleryContents( NULL ),
          mPriceList( NULL ),
          mMenuButton( mainFont, 4, -4, translate( "returnMenu" ) ),
          mReturnToMenu( false ) {

    addComponent( &mMenuButton );
    mMenuButton.addActionListener( this );
    }


        
CheckoutHousePage::~CheckoutHousePage() {
    if( mWebRequest != -1 ) {
        clearWebRequestSerial( mWebRequest );
        }
    if( mWifeName != NULL ) {
        delete [] mWifeName;
        }
    if( mSonName != NULL ) {
        delete [] mSonName;
        }
    if( mDaughterName != NULL ) {
        delete [] mDaughterName;
        }
    if( mHouseMap != NULL ) {
        delete [] mHouseMap;
        }
    if( mVaultContents != NULL ) {
        delete [] mVaultContents;
        }
    if( mBackpackContents != NULL ) {
        delete [] mBackpackContents;
        }
    if( mGalleryContents != NULL ) {
        delete [] mGalleryContents;
        }
    if( mPriceList != NULL ) {
        delete [] mPriceList;
        }
    }



char CheckoutHousePage::getReturnToMenu() {
    return mReturnToMenu;
    }



char *CheckoutHousePage::getWifeName() {
    if( mWifeName == NULL ) {
        return NULL;
        }
    else {
        return stringDuplicate( mWifeName );
        }
    }


char *CheckoutHousePage::getSonName() {
    if( mSonName == NULL ) {
        return NULL;
        }
    else {
        return stringDuplicate( mSonName );
        }
    }


char *CheckoutHousePage::getDaughterName() {
    if( mDaughterName == NULL ) {
        return NULL;
        }
    else {
        return stringDuplicate( mDaughterName );
        }
    }


int CheckoutHousePage::getPaymentCount() {
    return mPaymentCount;
    }

int CheckoutHousePage::getYouPaidTotal() {
    return mYouPaidTotal;
    }

int CheckoutHousePage::getWifePaidTotal() {
    return mWifePaidTotal;
    }

int CheckoutHousePage::getNumberOfTapes() {
    return mNumberOfTapes;
    }





char *CheckoutHousePage::getHouseMap() {
    if( mHouseMap == NULL ) {
        return NULL;
        }
    else {
        return stringDuplicate( mHouseMap );
        }
    }



char *CheckoutHousePage::getVaultContents() {
    if( mVaultContents == NULL ) {
        return NULL;
        }
    else {
        return stringDuplicate( mVaultContents );
        }
    }



char *CheckoutHousePage::getBackpackContents() {
    if( mBackpackContents == NULL ) {
        return NULL;
        }
    else {
        return stringDuplicate( mBackpackContents );
        }
    }



char *CheckoutHousePage::getGalleryContents() {
    if( mGalleryContents == NULL ) {
        return NULL;
        }
    else {
        return stringDuplicate( mGalleryContents );
        }
    }



char *CheckoutHousePage::getPriceList() {
    if( mPriceList == NULL ) {
        return NULL;
        }
    else {
        return stringDuplicate( mPriceList );
        }
    }



int CheckoutHousePage::getLootValue() {
    return mLootValue;
    }


char CheckoutHousePage::getMustSelfTest() {
    return mMustSelfTest;
    }


int CheckoutHousePage::getMusicSeed() {
    return mMusicSeed;
    }



void CheckoutHousePage::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == &mMenuButton ) {
        mReturnToMenu = true;
        }
    }


void CheckoutHousePage::step() {
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
                    mStatusMessageKey = "houseBeingRobbed";
                    mMenuButton.setVisible( true );
                    }
                else {
                    // house checked out!

                    SimpleVector<char *> *lines = 
                        tokenizeString( result );
                    
                    if( lines->size() != 16
                        ||
                        strcmp( *( lines->getElement( 15 ) ), "OK" ) != 0 ) {

                        setStatus( "err_badServerResponse", true );
                        mMenuButton.setVisible( true );
                    
                        for( int i=0; i<lines->size(); i++ ) {
                            delete [] *( lines->getElement( i ) );
                            }
                        }
                    else {
                        mHouseMap = *( lines->getElement( 0 ) );
                        mVaultContents = *( lines->getElement( 1 ) );
                        mBackpackContents = *( lines->getElement( 2 ) );
                        mGalleryContents = *( lines->getElement( 3 ) );
                        mPriceList = *( lines->getElement( 4 ) );
                        
                        mLootValue = 0;
                        sscanf( *( lines->getElement( 5 ) ),
                                "%d", &mLootValue );

                        int selfTestValue = 0;
                        sscanf( *( lines->getElement( 6 ) ),
                                "%d", &selfTestValue );

                        if( selfTestValue == 1 ) {
                            mMustSelfTest = true;
                            }
                        else {
                            mMustSelfTest = false;
                            }
                        
                        mMusicSeed = 0;
                        sscanf( *( lines->getElement( 7 ) ),
                                "%d", &mMusicSeed );
                        
                        mWifeName = *( lines->getElement( 8 ) );
                        mSonName = *( lines->getElement( 9 ) );
                        mDaughterName = *( lines->getElement( 10 ) );

                        mPaymentCount = 0;
                        sscanf( *( lines->getElement( 11 ) ),
                                "%d", &mPaymentCount );
                        mYouPaidTotal = 0;
                        sscanf( *( lines->getElement( 12 ) ),
                                "%d", &mYouPaidTotal );
                        mWifePaidTotal = 0;
                        sscanf( *( lines->getElement( 13 ) ),
                                "%d", &mWifePaidTotal );
                        
                        mNumberOfTapes = 0;
                        sscanf( *( lines->getElement( 14 ) ),
                                "%d", &mNumberOfTapes );


                        delete [] *( lines->getElement( 5 ) );
                        delete [] *( lines->getElement( 6 ) );
                        delete [] *( lines->getElement( 7 ) );
                        delete [] *( lines->getElement( 11 ) );
                        delete [] *( lines->getElement( 12 ) );
                        delete [] *( lines->getElement( 13 ) );
                        delete [] *( lines->getElement( 14 ) );
                        delete [] *( lines->getElement( 15 ) );
                        
                        printf( "HouseMap = %s\n", mHouseMap );
                        printf( "Vault = %s\n", mVaultContents );
                        printf( "Backpack = %s\n", mBackpackContents );
                        printf( "Gallery = %s\n", mGalleryContents );
                        printf( "PriceList = %s\n", mPriceList );
                        printf( "LootValue = %d\n", mLootValue );
                        printf( "MusicSeed = %d\n", mMusicSeed );
                        printf( "Wife = %s\n", mWifeName );
                        printf( "Son = %s\n", mSonName );
                        printf( "Daughter = %s\n", mDaughterName );
                        
                        // reset ping time, because house check-out
                        // counts as a ping
                        LiveHousePage::sLastPingTime = game_time( NULL );
                        }
                    delete lines;
                    }
                        
                        
                delete [] result;
                }
                break;
            }
        }
    }


        
void CheckoutHousePage::makeActive( char inFresh ) {
    if( !inFresh ) {
        return;
        }

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
    
    if( mHouseMap != NULL ) {
        delete [] mHouseMap;
        }
    mHouseMap = NULL;

    if( mVaultContents != NULL ) {
        delete [] mVaultContents;
        }
    mVaultContents = NULL;

    if( mBackpackContents != NULL ) {
        delete [] mBackpackContents;
        }
    mBackpackContents = NULL;

    if( mGalleryContents != NULL ) {
        delete [] mGalleryContents;
        }
    mGalleryContents = NULL;

    if( mPriceList != NULL ) {
        delete [] mPriceList;
        }
    mPriceList = NULL;
    
    char *ticketHash = getTicketHash();

    char *fullRequestURL = autoSprintf( 
        "%s?action=start_edit_house&user_id=%d"
        "&%s",
        serverURL, userID, ticketHash );
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

