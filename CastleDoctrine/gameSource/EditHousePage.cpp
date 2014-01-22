#include "EditHousePage.h"

#include "message.h"
#include "balance.h"


#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"
#include "minorGems/game/drawUtils.h"

#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SettingsManager.h"






extern Font *mainFont;


extern int diffHighlightsOff;



EditHousePage::EditHousePage() 
        : mStartHouseMap( NULL ),
          mVaultContents( NULL ),
          mBackpackContents( NULL ),
          mPriceList( NULL ),
          // starts empty
          mPurchaseList( stringDuplicate( "#" ) ),
          mSellList( stringDuplicate( "#" ) ),
          mObjectPicker( 8, 5 ),
          mGridDisplay( 0, 0, &mObjectPicker ),
          mDoneButton( mainFont, 8, -5, translate( "doneEdit" ) ),
          mBackpackButton( mainFont, 8, -3, translate( "loadBackpack" ) ),
          mAuctionButton( mainFont, -8, -5, translate( "openAuctionList" ) ),
          mUndoButton( mainFont, 8, -0.5, translate( "undo" ), 'z', 'Z' ),
          mJumpToTapesButton( mainFont, 8, -1.75, translate( "jumpToTapes" ) ),
          mSuicideButton( mainFont, 8, -0.5, translate( "suicide" ) ),
          mSuicideConfirmCheckbox( 8, .375, 1/16.0 ),
          mDiffHighlightToggleButton( "diffHighlightsOn.tga", 
                                      "diffHighlightsOff.tga", 
                                      8, -1.75, 1/16.0 ),
          mEyedropperStatus( 6.5, 5, 1/16.0 ),
          mBlockSuicideButton( false ),
          mGallery( mainFont, -8, 0 ),
          mNumberOfTapes( 0 ),
          mJumpToTapes( false ),
          mDone( false ),
          mDead( false ) {

    addComponent( &mDoneButton );
    addComponent( &mJumpToTapesButton );
    addComponent( &mSuicideButton );
    addComponent( &mSuicideConfirmCheckbox );
    addComponent( &mBackpackButton );
    addComponent( &mAuctionButton );
    addComponent( &mUndoButton );
    addComponent( &mGridDisplay );
    addComponent( &mObjectPicker );

    addComponent( &mDiffHighlightToggleButton );
    
    addComponent( &mEyedropperStatus );

    mDoneButton.setMouseOverTip( "" );
    mUndoButton.setMouseOverTip( translate( "undoTip" ) );
    mBackpackButton.setMouseOverTip( translate( "loadBackpackTip" ) );
    mAuctionButton.setMouseOverTip( translate( "openAuctionListTip" ) );

    mJumpToTapesButton.setMouseOverTip( translate( "jumpToTapesTip" ) );
    
    mSuicideButton.setMouseOverTip( translate( "unconfirmedSuicideTip" ) );
    mSuicideConfirmCheckbox.setMouseOverTip( 
        translate( "suicideConfirmTip" ) );
    mSuicideConfirmCheckbox.setMouseOverTipB( 
        translate( "suicideConfirmTip" ) );

    mDoneButton.addActionListener( this );
    
    mJumpToTapesButton.addActionListener( this );
    mSuicideButton.addActionListener( this );
    mSuicideConfirmCheckbox.addActionListener( this );
    mBackpackButton.addActionListener( this );
    mAuctionButton.addActionListener( this );
    mUndoButton.addActionListener( this );
    mUndoButton.setVisible( false );
    mGridDisplay.addActionListener( this );
    mObjectPicker.addActionListener( this );


    mDiffHighlightToggleButton.addActionListener( this );

    mDiffHighlightToggleButton.setMouseOverTip( 
        translate( "diffHighlightsOff" ) );
    mDiffHighlightToggleButton.setMouseOverTipB( 
        translate( "diffHighlightsOn" ) );
    
    
    
    addComponent( &mGallery );
    }


        
EditHousePage::~EditHousePage() {
    
    if( mStartHouseMap != NULL ) {
        delete [] mStartHouseMap;
        }

    if( mVaultContents != NULL ) {
        delete [] mVaultContents;
        }

    if( mBackpackContents != NULL ) {
        delete [] mBackpackContents;
        }

    if( mPriceList != NULL ) {
        delete [] mPriceList;
        }

    if( mPurchaseList != NULL ) {
        delete [] mPurchaseList;
        }
    if( mSellList != NULL ) {
        delete [] mSellList;
        }
    }



void EditHousePage::setWifeName( const char *inWifeName ) {
    mGridDisplay.setWifeName( inWifeName );
    mObjectPicker.setWifeName( inWifeName );
    }

void EditHousePage::setSonName( const char *inSonName ) {
    mGridDisplay.setSonName( inSonName );
    }

void EditHousePage::setDaughterName( const char *inDaughterName ) {
    mGridDisplay.setDaughterName( inDaughterName );
    }


char *EditHousePage::getWifeName() {
    return mGridDisplay.getWifeName();
    }

char *EditHousePage::getSonName() {
    return mGridDisplay.getSonName();
    }

char *EditHousePage::getDaughterName() {
    return mGridDisplay.getDaughterName();
    }



void EditHousePage::setHouseMap( const char *inHouseMap ) {
    if( mStartHouseMap != NULL ) {
        delete [] mStartHouseMap;
        }
    mStartHouseMap = stringDuplicate( inHouseMap );
    
    mGridDisplay.setHouseMap( inHouseMap );

    mUndoButton.setVisible( mGridDisplay.canUndo() );

    mDoneButton.setVisible( 
        mGridDisplay.areMandatoriesPlaced()
        &&
        mGridDisplay.doAllFamilyObjectsHaveExitPath() );
    
    mChangesCost = 0;
    mDiffHighlightToggleButton.setVisible( false );

    mMapStartedOutEmpty = mGridDisplay.getMapStartedOutEmpty();

    mBackpackOrVaultChanged = false;
    }



char *EditHousePage::getHouseMap() {
    return mGridDisplay.getHouseMap();
    }



void EditHousePage::setVaultContents( const char *inVaultContents ) {
    if( mVaultContents != NULL ) {
        delete [] mVaultContents;
        }
    mVaultContents = stringDuplicate( inVaultContents );
    }



char *EditHousePage::getVaultContents() {
    return stringDuplicate( mVaultContents );
    }



void EditHousePage::setBackpackContents( const char *inBackpackContents ) {
    if( mBackpackContents != NULL ) {
        delete [] mBackpackContents;
        }
    mBackpackContents = stringDuplicate( inBackpackContents );
    }



char *EditHousePage::getBackpackContents() {
    return stringDuplicate( mBackpackContents );
    }



void EditHousePage::setGalleryContents( const char *inGalleryContents ) {
    
    mGallery.setGalleryContents( inGalleryContents );
    }


void EditHousePage::setNumberOfTapes( int inNumber ) {
    mNumberOfTapes = inNumber;
    }


char *EditHousePage::getGalleryContents() {
    return mGallery.getGalleryContents();
    }





char *EditHousePage::getEditList() {
    return mGridDisplay.getEditList();
    }

char *EditHousePage::getFamilyExitPaths() {
    return mGridDisplay.getFamilyExitPaths();
    }


char EditHousePage::getWifeLiving() {
    return mGridDisplay.getWifeLiving();
    }


char *EditHousePage::getPurchaseList() {
    return stringDuplicate( mPurchaseList );
    }


void EditHousePage::setPurchaseList( const char *inPurchaseList ) {
    if( mPurchaseList != NULL ) {
        delete [] mPurchaseList;
        }
    mPurchaseList = stringDuplicate( inPurchaseList );
    }




char *EditHousePage::getSellList() {
    return stringDuplicate( mSellList );
    }


void EditHousePage::setSellList( const char *inSellList ) {
    if( mSellList != NULL ) {
        delete [] mSellList;
        }
    mSellList = stringDuplicate( inSellList );
    }




void EditHousePage::setPriceList( const char *inPriceList ) {
    if( mPriceList != NULL ) {
        delete [] mPriceList;
        }
    mPriceList = stringDuplicate( inPriceList );

    mObjectPicker.setPriceList( inPriceList );
    }



char *EditHousePage::getPriceList() {
    return stringDuplicate( mPriceList );
    }



void EditHousePage::setLootValue( int inLootValue ) {
    mLootValue = inLootValue;
    mBlockSuicideButton = false;

    checkIfPlacementAllowed();
    }



void EditHousePage::setMustSelfTest( char inMustSelfTest ) {
    mMustSelfTest = inMustSelfTest;
    
    // if house damaged and needs a self test, don't allow auctions
    // (server blocks auction activities anyway in this case)
    mAuctionButton.setVisible( !mMustSelfTest );
    }



void EditHousePage::checkIfPlacementAllowed() {
    // always allow placement with new accounting method
    mGridDisplay.allowPlacement( true );
        
    // can't afford to place anything, house not edited yet
    // allow suicide
    mSuicideButton.setVisible(
        ! mBlockSuicideButton &&
        ! mUndoButton.isVisible() &&
        mLootValue == 0 );

    if( mSuicideButton.isVisible() ) {
        mSuicideConfirmCheckbox.setVisible( true );
        mSuicideConfirmCheckbox.setToggled( false );
        mSuicideButton.setMouseOverTip( translate( "unconfirmedSuicideTip" ) );
        }
    else {
        mSuicideConfirmCheckbox.setVisible( false );
        }

    checkIfTapesButtonVisible();
    }



void EditHousePage::checkIfDoneButtonVisible() {
    // can't click DONE if house has no goal set
    // or family blocked
    // or spent more than we have on changes to house
    mDoneButton.setVisible( 
        mGridDisplay.areMandatoriesPlaced()
        &&
        mGridDisplay.doAllFamilyObjectsHaveExitPath()
        &&
        mLootValue >= mChangesCost );
    }



char EditHousePage::houseMapChanged() {

    char *editList = getEditList();
    
    int comp = strcmp( editList, "" );
    
    delete [] editList;

    if( comp != 0 ) {
        // some edits to report, whether or not map was actually changed
        // by edits, count it as changed
        return true;
        }
    

    if( mStartHouseMap == NULL || mMustSelfTest ) {
        return true;
        }

    char *newMap = mGridDisplay.getHouseMap();

    comp = strcmp( newMap, mStartHouseMap );

    if( comp != 0 ) {
        
        printf( "House maps differ.  Old:\n%s\n\nNew:\n%s\n\n", mStartHouseMap,
                newMap );
        }
    
    
    delete [] newMap;
    
    if( comp == 0 ) {
        return false;
        }


    return true;
    }




void EditHousePage::recomputeChangeCost() {
    mChangesCost = 0;
        
    SimpleVector<GridDiffRecord> diffList = mGridDisplay.getEditDiff();
    
    int numRecords = diffList.size();
    
    for( int i=0; i<numRecords; i++ ) {
        GridDiffRecord *r = diffList.getElement( i );
        
        mChangesCost += 
            r->placementCount *
            mObjectPicker.getPrice( r->objectID );
        }
        
    mGridDisplay.setTouchedHighlightRed( mChangesCost > mLootValue );

    mDiffHighlightToggleButton.setVisible( !mMapStartedOutEmpty && 
                                           mChangesCost > 0 );
    }


    

void EditHousePage::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == &mSuicideConfirmCheckbox ) {
        if( mSuicideConfirmCheckbox.getToggled() ) {
            mSuicideButton.setMouseOverTip( 
                translate( "suicideTip" ) );
            }
        else {
            mSuicideButton.setMouseOverTip( 
                translate( "unconfirmedSuicideTip" ) );
            }    
        }
    else if( inTarget == &mGridDisplay ) {
        int cost = 
            mObjectPicker.getPrice( mGridDisplay.getLastPlacedObject() );

        mUndoButton.setVisible( mGridDisplay.canUndo() );
        
        if( cost != -1 && ! mGridDisplay.wasLastActionPlayerMotion() ) {
            mObjectPicker.useSelectedObject();

            checkIfPlacementAllowed();
            }

        if( mGridDisplay.didLastActionChangeDiff() ) {    
            recomputeChangeCost();
            }
        
        checkIfDoneButtonVisible();

        // change to house map
        actionHappened();
        }
    else if( inTarget == &mDiffHighlightToggleButton ) {
        diffHighlightsOff = mDiffHighlightToggleButton.getToggled();
        
        SettingsManager::setSetting( "diffHighlightsOff", diffHighlightsOff );
        mGridDisplay.toggleTouchedHighlights( ! diffHighlightsOff );
        }
    else if( inTarget == &mBackpackButton ) {
        mShowLoadBackpack = true;
        }
    else if( inTarget == &mAuctionButton ) {
        mShowAuctions = true;
        }
    else if( inTarget == &mJumpToTapesButton ) {
        mJumpToTapes = true;
        }
    else if( inTarget == &mDoneButton ) {
        
        // Reset any states
        // that were toggled by the last robber.

        // We show the house-as-robbed view to the owner once,
        // until they perform their first complete edit, and THEN toggle
        // everything back.

        mGridDisplay.resetToggledStates( 0 );


        mDone = true;
        }
    else if( inTarget == &mSuicideButton ) {
        if( mSuicideConfirmCheckbox.isVisible() && 
            ! mSuicideConfirmCheckbox.getToggled() ) {
            return;
            }
        mGridDisplay.resetToggledStates( 0 );

        mDead = true;
        mDone = true;
        }
    else if( inTarget == &mObjectPicker ) {
        if( mObjectPicker.shouldShowGridView() ) {
            mShowGridObjectPicker = true;
            }
        else {
            // change in picked object
            checkIfPlacementAllowed();
            }
        }
    else if( inTarget == &mUndoButton ) {
        
        mBlockSuicideButton = true;

        mGridDisplay.undo();
        
        
        mUndoButton.setVisible( mGridDisplay.canUndo() );

        checkIfPlacementAllowed();
                
        recomputeChangeCost();
        checkIfDoneButtonVisible();

        // change to house map
        actionHappened();
        }    
    
    }





        
void EditHousePage::makeActive( char inFresh ) {
    LiveHousePage::makeActive( inFresh );
    
    if( !inFresh ) {
        return;
        }
    
    mJumpToTapes = false;
    mDone = false;
    mDead = false;
    mShowLoadBackpack = false;
    mShowAuctions = false;
    mShowGridObjectPicker = false;

    checkIfDoneButtonVisible();
    checkIfTapesButtonVisible();

    blockQuitting( areRequestsPending() );

    mDiffHighlightToggleButton.setToggled( diffHighlightsOff );
    mGridDisplay.toggleTouchedHighlights( ! mMapStartedOutEmpty && 
                                          ! diffHighlightsOff );
    }



void EditHousePage::step() {
    LiveHousePage::step();
    
    
    checkIfTapesButtonVisible();
    
    blockQuitting( areRequestsPending() );
    }




void EditHousePage::checkIfTapesButtonVisible() {
    // can jump to tapes as long as no editing done yet and no purchase/sale
    // done yet (so nothing will be lost when we abandon the house edit)
    // AND no background requests to server are pending
    mJumpToTapesButton.setVisible( ! areRequestsPending() && 
                                   mNumberOfTapes > 0 &&
                                   ! mUndoButton.isVisible() &&
                                   ! mBackpackOrVaultChanged );
    }




extern Font *numbersFontFixed;


void EditHousePage::draw( doublePair inViewCenter, 
                               double inViewSize ) {
        
    doublePair labelPos = { 0, 6.75 };
    
    drawMessage( "editDescription", labelPos, false );    

    
    drawBalance( mLootValue, mChangesCost );



    if( ! mGridDisplay.doAllFamilyObjectsHaveExitPath() ) {
        
        // explanation for why Done button hidden

        doublePair buttonPos = mDoneButton.getPosition();
        
        buttonPos.y += 0.5;

        drawMessage( "familyExitMessage", buttonPos, true );
        }
    
    }




void EditHousePage::keyDown( unsigned char inASCII ) {
    if( inASCII == '+' ) {
        mGridDisplay.saveWholeMapImage();
        }
    
    }
