#include "AuctionPage.h"

#include "minorGems/game/game.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/game/Font.h"

#include "message.h"
#include "balance.h"
#include "ticketHash.h"
#include "galleryObjects.h"

#include "serialWebRequests.h"




extern Font *mainFont;

extern char *serverURL;

extern int userID;




AuctionPage::AuctionPage() 
        : mDoneButton( mainFont, 8, -5, translate( "doneEdit" ) ),
          mUpdateButton( mainFont, 0, -5, translate( "auctionUpdateButton" ) ),
          mBuyButton( mainFont, 5, -5, translate( "buyButton" ) ),
          mDisplayOffset( 0 ),
          mUpButton( "up.tga", 7, 1, 1/16.0 ),
          mDownButton( "down.tga", 7, -1, 1/16.0 ),
          mLootValue( -1 ),
          mWebRequest( -1 ),
          mSecondsUntilUpdate( -1 ),
          mBaseTimestamp( -1 ),
          mDone( false ),
          mBuyExecuted( false ), 
          mBoughtObjectID( -1 ),
          mForceRefresh( false ) {

    addComponent( &mDoneButton );
    addComponent( &mUpdateButton );
    addComponent( &mBuyButton );
    
    mDoneButton.addActionListener( this );
    mUpdateButton.addActionListener( this );
    mBuyButton.addActionListener( this );
    
    mDoneButton.setMouseOverTip( "" );
    mUpdateButton.setMouseOverTip( translate( "auctionUpdateButtonTip" ) );
    
    mUpdateButton.setVisible( false );
    mBuyButton.setVisible( false );

    
    doublePair slotCenter = { -4.5, 5 };
    
    int numAuctionRows = NUM_AUCTION_SLOTS / NUM_AUCTION_SLOTS_PER_ROW;
    
    int slot = 0;

        
    for( int r=0; r<numAuctionRows; r++ ) {
        slotCenter.x = -4.5;

        for( int i=0; i<NUM_AUCTION_SLOTS_PER_ROW; i++ ) {
            
            mAuctionSlots[slot] = 
                new GallerySlotButton( mainFont,
                                       slotCenter.x, slotCenter.y,
                                       1 / 32.0 );
            slotCenter.x += 3;
        
            addComponent( mAuctionSlots[slot] );
            mAuctionSlots[slot]->addActionListener( this );
            slot ++;
            }
        slotCenter.y -= 3.5;
        }

    // up/down button to right of top/bottom rows
    doublePair upPosition = 
        mAuctionSlots[ NUM_AUCTION_SLOTS_PER_ROW - 1 ]->getPosition();
    doublePair downPosition = 
        mAuctionSlots[ NUM_AUCTION_SLOTS - 1 ]->getPosition();
    
    upPosition.x += 2;
    downPosition.x += 2;
    
    mUpButton.setPosition( upPosition.x, upPosition.y );
    mDownButton.setPosition( downPosition.x, downPosition.y );
    
    addComponent( &mUpButton );
    addComponent( &mDownButton );
    
    mUpButton.addActionListener( this );
    mDownButton.addActionListener( this );
    setUpDownVisibility();
    }


        
AuctionPage::~AuctionPage() {
    if( mWebRequest != -1 ) {
        clearWebRequestSerial( mWebRequest );
        }


    for( int i=0; i<NUM_AUCTION_SLOTS; i++ ) {
        delete mAuctionSlots[i];
        }

    }



void AuctionPage::setUpDownVisibility() {
    mUpButton.setVisible( mDisplayOffset > 0 );
    
    mDownButton.setVisible( mFullIDList.size() - mDisplayOffset > 
                            NUM_AUCTION_SLOTS );
    }
    


void AuctionPage::populateSlots() {
    
    for( int i=0; i<NUM_AUCTION_SLOTS; i++ ) {
        mAuctionSlots[i]->setObject( -1 );
        mAuctionPrices[i] = -1;
        }
    turnAllRingsOff();
    mBuyButton.setVisible( false );

    int slotNumber = 0;
    for( int i=mDisplayOffset; 
         i<mFullIDList.size() && slotNumber < NUM_AUCTION_SLOTS; 
         i++ ) {
        
        mAuctionSlots[ slotNumber ]->setObject( 
            *( mFullIDList.getElement( i ) ) );

        mAuctionPrices[ slotNumber ] = *( mFullPriceList.getElement( i ) );
        
        slotNumber++;
        }
    }



char AuctionPage::getDone() {
    return mDone;
    }

char AuctionPage::getBuy() {
    return mBuyExecuted;
    }


int AuctionPage::getBoughtObject() {
    return mBoughtObjectID;
    }



void AuctionPage::setLootValue( int inLootValue ) {
    mLootValue = inLootValue;
    }




void AuctionPage::turnAllRingsOff() {
    for( int i=0; i<NUM_AUCTION_SLOTS; i++ ) {
        mAuctionSlots[i]->setRingOn( false );
        }
    }


char AuctionPage::getPricesStale() {
    if( mSecondsUntilUpdate <= 0 ) {
        return true;
        }
    if( ( game_time(NULL) - mBaseTimestamp ) >= mSecondsUntilUpdate ) {
        return true;
        }
    return false;
    }



void AuctionPage::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == &mDoneButton ) {
        mDone = true;
        }    
    else if( inTarget == &mUpdateButton ) {
        mUpdateButton.setVisible( false );
        setToolTip( "" );
        refreshPrices();
        
        // keep house checked out as long as user is still actively
        // watching the auctions with the Update button
        actionHappened();
        }
    else if( inTarget == &mBuyButton ) {
        for( int i=0; i<NUM_AUCTION_SLOTS; i++ ) {
            if( mAuctionSlots[i]->getRingOn() ) {
                mBoughtObjectID = mAuctionSlots[i]->getObject();
                mBuyExecuted = true;
                mForceRefresh = true;
                break;
                }
            }
        }
    else if( inTarget == &mUpButton ) {
        mDisplayOffset -= NUM_AUCTION_SLOTS;
        populateSlots();
        setUpDownVisibility();
        }
    else if( inTarget == &mDownButton ) {
        mDisplayOffset += NUM_AUCTION_SLOTS;
        populateSlots();
        setUpDownVisibility();
        }
    else if( !getPricesStale() ) {
        // don't allow user to pick items to buy if prices are stale

        for( int i=0; i<NUM_AUCTION_SLOTS; i++ ) {
            if( inTarget == mAuctionSlots[i] ) {
                
                char ringWasOn = mAuctionSlots[i]->getRingOn();

                turnAllRingsOff();
                
                int hitObject = mAuctionSlots[i]->getObject();
                

                if( !ringWasOn && 
                    hitObject != -1 &&
                    mAuctionPrices[i] <= mLootValue ) {
                    
                    mAuctionSlots[i]->setRingOn( true );
                    
                    mBuyButton.setVisible( true );
                    
                    const char *objectDescription = 
                        getGalleryObjectDescription( hitObject );
                    
                    char *quotedDescription = 
                        autoSprintf( "\"%s\"", objectDescription );
                    
                    

                    char *tipString = autoSprintf( 
                        translate( "buyButtonTip" ),
                        quotedDescription, mAuctionPrices[i] );;

                    if( strlen( tipString ) > 58 ) {    
                        delete [] tipString;
                        
                        tipString = autoSprintf( 
                            translate( "shortBuyButtonTip" ),
                            quotedDescription  );
                        }
                    
                    delete [] quotedDescription;

                    mBuyButton.setMouseOverTip( tipString );
                    
                    delete [] tipString;
                    }
                else {
                    mBuyButton.setVisible( false );
                    }
                

                break;
                }
            }
        
        }
    }



void AuctionPage::step() {
    LiveHousePage::step();
    
    if( mWebRequest != -1 ) {
            
        int stepResult = stepWebRequestSerial( mWebRequest );
    
        if( stepResult != 0 ) {
            setWaiting( false );
            mDoneButton.setVisible( true );
            }

        switch( stepResult ) {
            case 0:
                break;
            case -1:
                setStatus( "err_webRequest", true );
                clearWebRequestSerial( mWebRequest );
                mWebRequest = -1;
                break;
            case 1: {
                char *result = getWebResultSerial( mWebRequest );
                clearWebRequestSerial( mWebRequest );
                mWebRequest = -1;
                     
                printf( "Web result = %s\n", result );
   
                if( strstr( result, "DENIED" ) != NULL ) {
                    setStatus( "auctionListFetchFailed", true );
                    }
                else {
                    // got auction list!

                    setStatus( NULL, false );

                    // parse result
                    SimpleVector<char *> *lines = 
                        tokenizeString( result );
                    
                    
                    char badParse = false;

                    if( lines->size() < 2 ) {
                        // list could be empty, but it still must have 
                        // seconds_until_price_drop and 
                        // OK at the end
                        badParse = true;
                        }
                    else {
                        char *string = *( lines->getElement( 0 ) );
                        
                        // first line should be seconds until update
                        sscanf( string, "%d", &mSecondsUntilUpdate );
                        delete [] string;

                        if( mSecondsUntilUpdate > 0 ) {
                            mBaseTimestamp = game_time( NULL );
                            }
                        }
                        
                    int slotNumber = 0;

                    // last line should be OK
                    for( int i=1; i<lines->size() - 1; i++ ) {
                        char *line = *( lines->getElement( i ) );
                        
                        int numParts;
                        char **parts = split( line, "#", &numParts );
                        
                        if( numParts != 2 ) {
                            printf( "Unexpected number of parts on auction "
                                    "list line: %d\n", numParts );
                            badParse = true;
                            }
                        else {
                            
                            int id;
                            sscanf( parts[0], "%d", &id );
                            
                            int price;
                            sscanf( parts[1], "%d", &price );

                            
                            mFullIDList.push_back( id );
                            mFullPriceList.push_back( price );
                            
                            slotNumber ++;
                            }
                        
                        for( int j=0; j<numParts; j++ ) {
                            delete [] parts[j];
                            }
                        delete [] parts;

                        
                        delete [] line;
                        }
                    
                    if( lines->size() > 0 ) {
                        
                        char *line = 
                            *( lines->getElement( lines->size() -1  ) );
                    
                        if( strcmp( line, "OK" ) != 0 ) {
                            badParse = true;
                            }
                        
                        delete [] line;
                        }
                    
                    

                    delete lines;

                    
                    if( badParse ) {
                        setStatus( "auctionListFetchFailed", true );
                        }

                    populateSlots();
                    setUpDownVisibility();
                    }
                        
                        
                delete [] result;
                }
                break;
            }
        return;
        }

    
    // else no web request

    if( getPricesStale() ) {
        // expired
        if( ! isStatusShowing() ) {
            mUpdateButton.setVisible( true );
            }

        turnAllRingsOff();
        mBuyButton.setVisible( false );
        }
    }


        
void AuctionPage::draw( doublePair inViewCenter, 
                          double inViewSize ) {
    
    doublePair labelPos = { 0, 6.75 };
    
    drawMessage( "auctionDescription", labelPos, false );

    labelPos.y = 0;

    for( int i=0; i<NUM_AUCTION_SLOTS; i++ ) {
        if( mAuctionPrices[i] > 0 ) {
        
            char *priceString = autoSprintf( "$%d", mAuctionPrices[i] );
            doublePair pricePos = mAuctionSlots[i]->getCenter();
            pricePos.y -= 1.5625;
            
            drawMessage( priceString, pricePos, false );

            delete [] priceString;
            }
        }
    
    if( mSecondsUntilUpdate > 0 ) {
        
        int currentTime = game_time( NULL );
        
        int timeLeft = mSecondsUntilUpdate - ( currentTime - mBaseTimestamp );
        
        
        // draw time where Update button will eventually go
        labelPos = mUpdateButton.getCenter();

        if( timeLeft > 0 ) {
            
            int minutesLeft = timeLeft / 60;
            int secondsLeft = timeLeft - minutesLeft * 60;
            
            

            char *timeMessage = autoSprintf( "%d:%02d",
                                             minutesLeft, secondsLeft );
            
            mainFont->drawString( timeMessage, labelPos, alignLeft );
        
            delete [] timeMessage;
            }
        

        labelPos.x -= 2;

        if( timeLeft > 0 ) {
            mainFont->drawString( translate( "auctionTimeString" ),
                                  labelPos, alignRight );
            }
        else {
            mainFont->drawString( translate( "auctionPricesStale" ), 
                                  labelPos, alignRight );
            }
        }



    drawBalance( mLootValue, 0 );
    }


        
void AuctionPage::makeActive( char inFresh ) {
    LiveHousePage::makeActive( inFresh );
    
    if( !inFresh ) {
        return;
        }

    mDone = false;
    mBuyExecuted = false;
    mBoughtObjectID = -1;
    
    if( mWebRequest == -1 ) {
        if( mForceRefresh || mSecondsUntilUpdate <= 0 || getPricesStale() ) {
            
            refreshPrices();
            }
        // else keep existing prices (not stale)
        }
    // else go with prices that will be fetched by current web request
    }



void AuctionPage::refreshPrices() {
    
    mForceRefresh = false;

    if( mWebRequest != -1 ) {
        clearWebRequestSerial( mWebRequest );
        }

    mDisplayOffset = 0;
    mFullIDList.deleteAll();
    mFullPriceList.deleteAll();

    // populate slots with empty list to clear them
    populateSlots();
    setUpDownVisibility();
    
    
    mUpdateButton.setVisible( false );

    mDoneButton.setVisible( false );
    
    mSecondsUntilUpdate = -1;
    mBaseTimestamp = -1;
    

    // request house list from server
    char *ticketHash = getTicketHash();
    
    char *actionString = autoSprintf( "action=list_auctions&user_id=%d&%s", 
                                      userID, ticketHash );
    delete [] ticketHash;
            
    
    mWebRequest = startWebRequestSerial( "POST", 
                                   serverURL, 
                                   actionString );

    delete [] actionString;

    setWaiting( true );
    }


        

