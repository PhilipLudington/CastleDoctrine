#include "CheckinHousePage.h"
#include "ticketHash.h"
#include "message.h"

#include "serialWebRequests.h"


#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"

#include "minorGems/util/stringUtils.h"


extern Font *mainFont;


extern char *serverURL;

extern int userID;



CheckinHousePage::CheckinHousePage() 
        : mRequestStarted( false ),
          mWebRequest( -1 ),
          mHouseMap( NULL ),
          mVaultContents( NULL ),
          mBackpackContents( NULL ),
          mGalleryContents( NULL ),
          mFamilyExitPaths( NULL ),
          mPurchaseList( NULL ),
          mSellList( NULL ),
          mPriceList( NULL ),
          mMoveList( NULL ),
          mDied( 0 ),
          mMenuButton( mainFont, 4, -4, translate( "returnMenu" ) ),
          mStartOverButton( mainFont, 4, -4, translate( "startOver" ) ),
          mReturnToMenu( false ),
          mStartOver( true ) {

    addComponent( &mMenuButton );
    mMenuButton.addActionListener( this );

    addComponent( &mStartOverButton );
    mStartOverButton.addActionListener( this );
    }


        
CheckinHousePage::~CheckinHousePage() {
    if( mWebRequest != -1 ) {
        clearWebRequestSerial( mWebRequest );
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
    if( mFamilyExitPaths != NULL ) {
        delete [] mFamilyExitPaths;
        }
    if( mPurchaseList != NULL ) {
        delete [] mPurchaseList;
        }
    if( mSellList != NULL ) {
        delete [] mSellList;
        }
    if( mPriceList != NULL ) {
        delete [] mPriceList;
        }
    if( mMoveList != NULL ) {
        delete [] mMoveList;
        }
    }



char CheckinHousePage::getReturnToMenu() {
    return mReturnToMenu;
    }


char CheckinHousePage::getStartOver() {
    return mStartOver;
    }



void CheckinHousePage::setHouseMap( const char *inHouseMap ) {
    if( mHouseMap != NULL ) {
        delete [] mHouseMap;
        }
    mHouseMap = stringDuplicate( inHouseMap );
    }



void CheckinHousePage::setVaultContents( const char *inVaultContents ) {
    if( mVaultContents != NULL ) {
        delete [] mVaultContents;
        }
    mVaultContents = stringDuplicate( inVaultContents );
    }



void CheckinHousePage::setBackpackContents( const char *inBackpackContents ) {
    if( mBackpackContents != NULL ) {
        delete [] mBackpackContents;
        }
    mBackpackContents = stringDuplicate( inBackpackContents );
    }



void CheckinHousePage::setGalleryContents( const char *inGalleryContents ) {
    if( mGalleryContents != NULL ) {
        delete [] mGalleryContents;
        }
    mGalleryContents = stringDuplicate( inGalleryContents );
    }



void CheckinHousePage::setFamilyExitPaths( const char *inFamilyExitPaths ) {
    if( mFamilyExitPaths != NULL ) {
        delete [] mFamilyExitPaths;
        }
    mFamilyExitPaths = stringDuplicate( inFamilyExitPaths );
    }



void CheckinHousePage::setPurchaseList( const char *inPurchaseList ) {
    if( mPurchaseList != NULL ) {
        delete [] mPurchaseList;
        }
    mPurchaseList = stringDuplicate( inPurchaseList );
    }


void CheckinHousePage::setSellList( const char *inSellList ) {
    if( mSellList != NULL ) {
        delete [] mSellList;
        }
    mSellList = stringDuplicate( inSellList );
    }



void CheckinHousePage::setPriceList( const char *inPriceList ) {
    if( mPriceList != NULL ) {
        delete [] mPriceList;
        }
    mPriceList = stringDuplicate( inPriceList );
    }


void CheckinHousePage::setMoveList( const char *inMoveList ) {
    if( mMoveList != NULL ) {
        delete [] mMoveList;
        }
    mMoveList = stringDuplicate( inMoveList );
    }



void CheckinHousePage::setDied( int inDied ) {
    mDied = inDied;
    }



void CheckinHousePage::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == &mMenuButton ) {
        mReturnToMenu = true;
        }
    else if( inTarget == &mStartOverButton ) {
        mStartOver = true;
        }
    }


void CheckinHousePage::step() {
    
    // wait until pending LiveHousePage requests are sent
    // to prevent bad request orderings (house checked in before 
    //  start_self_test received by server).
    if( LiveHousePage::areRequestsPending() ) {
        
        // note that we don't step LiveHousePage otherwise, because
        // we don't want it starting NEW ping requests after
        // we send our request (lest our request's sequence number
        // will be stale)
        LiveHousePage::step();

        return;
        }
    
    if( ! mRequestStarted ) {
        // send back to server            
        char *ticketHash = getTicketHash();
        
            
    
        char *actionString = autoSprintf( 
            "action=end_edit_house&user_id=%d"
            "&%s&died=%d&house_map=%s&vault_contents=%s"
            "&backpack_contents=%s&gallery_contents=%s"
            "&price_list=%s&purchase_list=%s&sell_list=%s"
            "&self_test_move_list=%s&family_exit_paths=%s",
            userID, ticketHash, mDied, mHouseMap, mVaultContents, 
            mBackpackContents, mGalleryContents, 
            mPriceList, mPurchaseList, mSellList, mMoveList,
            mFamilyExitPaths );
        delete [] ticketHash;
            
    
        mWebRequest = startWebRequestSerial( "POST", 
                                       serverURL, 
                                       actionString );
    
        delete [] actionString;

        mRequestStarted = true;
        
        return;
        }
    


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
   
                if( strstr( result, "OK" ) != NULL ) {
                    // house checked in!
                    
                    if( mDied != 1 ) {    
                        mStatusError = false;
                        mStatusMessageKey = "houseCheckedIn";
                        mReturnToMenu = true;
                        }
                    else {
                        mStatusError = true;
                        mStatusMessageKey = "deathMessage";
                        mStartOverButton.setVisible( true );
                        blockQuitting( false );
                        }
                    }
                else {
                    mStatusError = true;
                    mStatusMessageKey = "houseCheckInFailed";
                    mMenuButton.setVisible( true );
                    blockQuitting( false );
                    }
                
                        
                        
                delete [] result;
                }
                break;
            }
        }
    }



        
void CheckinHousePage::makeActive( char inFresh ) {
    LiveHousePage::makeActive( inFresh );

    if( !inFresh ) {
        return;
        }
    
    
    mRequestStarted = false;

    mReturnToMenu = false;
    mStartOver = false;
    
    mStatusError = false;
    mStatusMessageKey = NULL;

    mMenuButton.setVisible( false );
    mStartOverButton.setVisible( false );
    
    setWaiting( true );
    }


