#include "BuyAuctionPage.h"
#include "ticketHash.h"
#include "message.h"
#include "galleryObjects.h"

#include "serialWebRequests.h"


#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"

#include "minorGems/util/stringUtils.h"


extern Font *mainFont;


extern char *serverURL;

extern int userID;


#define SLOT_Y 5.5


BuyAuctionPage::BuyAuctionPage() 
        : mWebRequest( -1 ),
          mGalleryContents( NULL ),
          mLootValue( -1 ),
          mHomeButton( mainFont, 4, -4, translate( "returnHome" ) ),
          mBoughtSlot( mainFont, 0, 0, 1/32.0 ),
          mReturnToHome( false ) {

    addComponent( &mHomeButton );
    mHomeButton.addActionListener( this );

    mHomeButton.setMouseOverTip( "" );

    addComponent( &mBoughtSlot );

    mBoughtSlot.setVisible( false );
    }


        
BuyAuctionPage::~BuyAuctionPage() {
    if( mWebRequest != -1 ) {
        clearWebRequestSerial( mWebRequest );
        }
    if( mGalleryContents != NULL ) {
        delete [] mGalleryContents;
        }
    }



char BuyAuctionPage::getReturnToHome() {
    return mReturnToHome;
    }



void BuyAuctionPage::setGalleryContents( const char *inGalleryContents ) {
    if( mGalleryContents != NULL ) {
        delete [] mGalleryContents;
        }
    mGalleryContents = stringDuplicate( inGalleryContents );
    }



char *BuyAuctionPage::getGalleryContents() {
    if( mGalleryContents != NULL ) {
        return stringDuplicate( mGalleryContents );
        }

    return NULL;
    }

void BuyAuctionPage::setObject( int inObjectID ) {
    mObjectID = inObjectID;
    }



void BuyAuctionPage::setLootValue( int inLootValue ) {
    mLootValue = inLootValue;
    }


int BuyAuctionPage::getLootValue() {
    return mLootValue;
    }



void BuyAuctionPage::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == &mHomeButton ) {
        mReturnToHome = true;
        }
    }


void BuyAuctionPage::step() {
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
                mHomeButton.setVisible( true );
                break;
            case 1: {
                char *result = getWebResultSerial( mWebRequest );
                clearWebRequestSerial( mWebRequest );
                mWebRequest = -1;
                     
                printf( "Web result = %s\n", result );
   
                if( strstr( result, "DENIED" ) != NULL ) {
                    mStatusError = true;
                    mStatusMessageKey = "auctionBuyFailed";
                    mHomeButton.setVisible( true );
                    }
                else {
                    // auction successful
                    
                    SimpleVector<char *> *tokens =
                        tokenizeString( result );
                    
                    if( tokens->size() != 2 ||
                        strcmp( *( tokens->getElement( 1 ) ), "OK" ) != 0 ) {
                        mStatusError = true;
                        mStatusMessageKey = "err_badServerResponse";
                        }
                    else {
                        int price;
                        sscanf( *( tokens->getElement( 0 ) ), 
                                "%d", &price );
                        
                        mBoughtSlot.setObject( mObjectID );
                        mBoughtSlot.setVisible( true );
                        
                        mLootValue -= price;
                        
                        if( strcmp( mGalleryContents, "#" ) == 0 ) {
                            // was empty, just the one new item now
                            delete [] mGalleryContents;
                            mGalleryContents = autoSprintf( "%d", mObjectID );
                            }
                        else {
                            // prepend
                            char *old = mGalleryContents;
                            
                            mGalleryContents = autoSprintf( "%d#%s",
                                                            mObjectID,
                                                            mGalleryContents );
                            delete [] old;
                            }
                        
                        
                        const char *objectDescription = 
                            getGalleryObjectDescription( mObjectID );
                    
                        char *quotedDescription = 
                            autoSprintf( "\"%s\"", objectDescription );
                    


                        char *statusString = autoSprintf( 
                            translate( "auctionBuySucceed" ),
                            quotedDescription, price );
                    
                        delete [] quotedDescription;
                        
                        setStatusDirect( statusString, false );
                        
                        delete [] statusString;
                        }

                    mHomeButton.setVisible( true ); 

                    for( int i=0; i<tokens->size(); i++ ) {
                        delete [] *( tokens->getElement( i ) );
                        }
                    delete tokens;
                    }
                        
                        
                delete [] result;
                }
                break;
            }
        }
    }



        
void BuyAuctionPage::makeActive( char inFresh ) {
    if( !inFresh ) {
        return;
        }
    
    // send back to server            
    char *ticketHash = getTicketHash();
        
            
    
    char *actionString = autoSprintf( 
        "action=buy_auction&user_id=%d"
        "&%s"
        "&object_id=%d",
        userID, ticketHash, mObjectID );
    delete [] ticketHash;
            
    
    mWebRequest = startWebRequestSerial( "POST", 
                                   serverURL, 
                                   actionString );
    
    delete [] actionString;

    mReturnToHome = false;
    
    mStatusError = false;
    mStatusMessageKey = NULL;

    setStatusDirect( NULL, false );
    

    mHomeButton.setVisible( false );
    
    mBoughtSlot.setVisible( false );

    setWaiting( true );
    }




void BuyAuctionPage::draw( doublePair inViewCenter, 
                                double inViewSize ) {
    
    }


