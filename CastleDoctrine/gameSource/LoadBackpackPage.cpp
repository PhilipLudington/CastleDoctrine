#include "LoadBackpackPage.h"

#include "message.h"
#include "balance.h"
#include "tools.h"

#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"
#include "minorGems/game/drawUtils.h"

#include "minorGems/util/stringUtils.h"




extern Font *mainFont;





LoadBackpackPage::LoadBackpackPage() 
        : mSellMode( false ),
          mSellHalfMode( false ),
          mMoveHalfMode( false ),
          mToolPicker( 8, 5, true ),
          mDoneButton( mainFont, 8, -5, translate( "doneEdit" ) ),
          mSellModeButton( mainFont, 8, -2.5, translate( "sellMode" ) ),
          mSellHalfButton( mainFont, 5, -4, translate( "sellHalf" ) ),
          mSellOneButton( mainFont, 5, -5, translate( "sellOne" ) ),
          mSellAllButton( mainFont, 5, 0, translate( "sellAll" ) ),
          mBuyModeButton( mainFont, 8, -1.5, translate( "buyMode" ) ),
          mMoveHalfButton( mainFont, 5, -4, translate( "moveHalf" ) ),
          mMoveOneButton( mainFont, 5, -5, translate( "moveOne" ) ),
          mUndoButton( mainFont, 8, 0, translate( "undo" ), 'z', 'Z' ),
          // lie about sprite size and set sprite below, so border
          // isn't so huge
          mBuyButton( NULL, 9, 9, 5, 5, 1 / 16.0 ),
          mDone( false ),
          mChangeHappened( false ) {

    mBuyButton.setSprite( loadSprite( "buy.tga" ), true );


    addComponent( &mDoneButton );
    addComponent( &mUndoButton );
    addComponent( &mBuyButton );

    addComponent( &mSellModeButton );
    addComponent( &mSellHalfButton );
    addComponent( &mSellOneButton );
    addComponent( &mSellAllButton );

    addComponent( &mBuyModeButton );
    addComponent( &mMoveHalfButton );
    addComponent( &mMoveOneButton );

    addComponent( &mToolPicker );

    
    mSellModeButton.setMouseOverTip( translate( "sellModeTip" ) );
    mSellHalfButton.setMouseOverTip( translate( "sellHalfTip" ) );
    mSellOneButton.setMouseOverTip( translate( "sellOneTip" ) );
    
    mBuyModeButton.setMouseOverTip( translate( "buyModeTip" ) );

    mMoveHalfButton.setMouseOverTip( translate( "moveHalfTip" ) );
    mMoveOneButton.setMouseOverTip( translate( "moveOneTip" ) );
    

    mDoneButton.setMouseOverTip( "" );

    mDoneButton.addActionListener( this );
    mUndoButton.addActionListener( this );
    mUndoButton.setVisible( false );
    
    mSellModeButton.addActionListener( this );
    mSellHalfButton.addActionListener( this );
    mSellOneButton.addActionListener( this );
    mSellAllButton.addActionListener( this );

    mBuyModeButton.addActionListener( this );
    mBuyModeButton.setVisible( false );

    mMoveHalfButton.addActionListener( this );
    mMoveOneButton.addActionListener( this );


    mToolPicker.addActionListener( this );


    mBuyButton.addActionListener( this );
    mBuyButton.setVisible( false );
    
    doublePair slotCenter = { -8, 5 };

    for( int i=0; i<NUM_PACK_SLOTS; i++ ) {
        mPackSlots[i] = new InventorySlotButton( mainFont, 
                                                 slotCenter.x, slotCenter.y,
                                                 1 / 32.0 );        
        slotCenter.x += 1.5;
        
        addComponent( mPackSlots[i] );
        mPackSlots[i]->addActionListener( this );
        }

    
    slotCenter.x = -8;
    slotCenter.y = 2;

    int numVaultRows = NUM_VAULT_SLOTS / NUM_PACK_SLOTS;
    
    int slot = 0;
    
    for( int r=0; r<numVaultRows; r++ ) {
        slotCenter.x = -8;

        for( int i=0; i<NUM_PACK_SLOTS; i++ ) {
            
            mVaultSlots[slot] = 
                new InventorySlotButton( mainFont, slotCenter.x, slotCenter.y,
                                         1 / 32.0 );
            slotCenter.x += 1.5;
        
            addComponent( mVaultSlots[slot] );
            mVaultSlots[slot]->addActionListener( this );
            slot ++;
            }
        slotCenter.y -= 1.5;
        }

    // for testing
    //mPackSlots[0]->setObject( 500 );
    //mPackSlots[2]->setObject( 501 );
    

    checkSellModeStatus();
    }


        
LoadBackpackPage::~LoadBackpackPage() {

    for( int i=0; i<NUM_PACK_SLOTS; i++ ) {
        delete mPackSlots[i];
        }

    for( int i=0; i<NUM_VAULT_SLOTS; i++ ) {
        delete mVaultSlots[i];
        }
    }



void LoadBackpackPage::setVaultContents( char *inVaultContents ) {
    inventorySlotsFromString( inVaultContents, mVaultSlots, NUM_VAULT_SLOTS );
    mChangeHappened = false;
    }



char *LoadBackpackPage::getVaultContents() {

    return stringFromInventorySlots( mVaultSlots, NUM_VAULT_SLOTS );
    }



void LoadBackpackPage::setBackpackContents( char *inBackpackContents ) {

    inventorySlotsFromString( inBackpackContents,
                              mPackSlots, NUM_PACK_SLOTS );
       
    checkBuyButtonStatus();
    }



char *LoadBackpackPage::getBackpackContents() {

    return stringFromInventorySlots( mPackSlots, NUM_PACK_SLOTS );
    }




char *LoadBackpackPage::getPurchaseList() {
    
    return toString( &mPurchaseRecords );
    }


void LoadBackpackPage::setPurchaseList( char *inPurchaseList ) {
    mPurchaseRecords.deleteAll();
    
    fromString( inPurchaseList, &mPurchaseRecords );

    // this is a fresh loading session
    // undoing old purchase history here would be confusing (and sometimes
    // incorrect)
    mPurchaseHistory.deleteAll();
    mUndoButton.setVisible( false );
    }



char *LoadBackpackPage::getSellList() {
    
    return toString( &mSellRecords );
    }


void LoadBackpackPage::setSellList( char *inSellList ) {
    mSellRecords.deleteAll();
    
    fromString( inSellList, &mSellRecords );

    // this is a fresh loading session
    // undoing old purchase history here would be confusing (and sometimes
    // incorrect)
    mPurchaseHistory.deleteAll();
    mUndoButton.setVisible( false );
    }



void LoadBackpackPage::setPriceList( char *inPriceList ) {
    mToolPicker.setPriceList( inPriceList );
       
    checkBuyButtonStatus();
    }



void LoadBackpackPage::setLootValue( int inLootValue ) {
    mLootValue = inLootValue;

    checkBuyButtonStatus();
    }



void LoadBackpackPage::checkBuyButtonStatus() {
    
    if( mSellMode ) {
        mBuyButton.setVisible( false );
        return;
        }

    int selectedObject = mToolPicker.getSelectedObject();
    
    if( selectedObject == -1 ) {
        mBuyButton.setVisible( false );
        return;
        }

    int price = mToolPicker.getPrice( selectedObject );

    if( price > mLootValue ) {
        mBuyButton.setVisible( false );
        return;
        }
    
    // we can afford it

    for( int i=0; i<NUM_PACK_SLOTS; i++ ) {
        int slotObject = mPackSlots[i]->getObject();
        
        if( slotObject == -1 || slotObject == selectedObject ) {
            // empty or matching slot found
            mBuyButton.setVisible( true );

            char *tip = autoSprintf( translate( "buyTip" ),
                                     getToolDescription( selectedObject ) );
        
            mBuyButton.setMouseOverTip( tip );
            delete [] tip;                

            return;
            }
        }
    // no empty/matching slots
    mBuyButton.setVisible( false );
    }



void LoadBackpackPage::checkUndoStatus() {
            
    if( mPurchaseHistory.size() == 0 ) {
        // history exhausted
        mUndoButton.setVisible( false );
        }
    else {
        mUndoButton.setVisible( true );
        
        PurchaseHistoryRecord lastTrans = 
            *( mPurchaseHistory.getElement( mPurchaseHistory.size() - 1 ) );

        
        char *tip;
        
        if( lastTrans.quantity < -1 ) {
            // sell multiple
            tip = 
                autoSprintf( translate( "backpackMultipleSellUndoTip" ), 
                             - lastTrans.quantity,
                             getToolDescriptionPlural( lastTrans.objectID ) );
            }
        else {
            // sell or buy single
            const char *tipKey = "backpackUndoTip";

            if( lastTrans.quantity < 0 ) {
                // last buy was a sell
                
                tipKey = "backpackSellUndoTip";
                }
            
            tip = autoSprintf( translate( tipKey ), 
                               getToolDescription( lastTrans.objectID ) );
            }
        
        mUndoButton.setMouseOverTip( tip );
        delete [] tip;
        }
    }



void LoadBackpackPage::checkSellModeStatus() {
    mSellModeButton.setVisible( !mSellMode );
    mBuyModeButton.setVisible( mSellMode );
    
    mSellHalfButton.setVisible( mSellMode && !mSellHalfMode );
    mSellOneButton.setVisible( mSellMode && mSellHalfMode );

    mMoveHalfButton.setVisible( !mSellMode && !mMoveHalfMode );
    mMoveOneButton.setVisible( !mSellMode && mMoveHalfMode );

    int packTransferMode = 1;
    int vaultTransferMode = 2;
    
    if( mSellMode ) {
        if( mSellHalfMode ) {
            packTransferMode = 4;
            vaultTransferMode = 4;
            }
        else {
            packTransferMode = 3;
            vaultTransferMode = 3;
            }
        }
    else if( mMoveHalfMode ) {
        packTransferMode = 5;
        vaultTransferMode = 6;
        }

    int totalResaleValue = 0;

    for( int i=0; i<NUM_PACK_SLOTS; i++ ) {
        mPackSlots[i]->setTransferStatus( packTransferMode );
        int id = mPackSlots[i]->getObject();
        
        if( id != -1 ) {
            mPackSlots[i]->setSellPrice( mToolPicker.getSellBackPrice( id ) );
            totalResaleValue += 
                mToolPicker.getSellBackPrice( id ) * 
                mPackSlots[i]->getQuantity();
            }
        }
    for( int i=0; i<NUM_VAULT_SLOTS; i++ ) {
        mVaultSlots[i]->setTransferStatus( vaultTransferMode );
        int id = mVaultSlots[i]->getObject();
        
        if( id != -1 ) {
            mVaultSlots[i]->setSellPrice( mToolPicker.getSellBackPrice( id ) );
            totalResaleValue += 
                mToolPicker.getSellBackPrice( id ) * 
                mVaultSlots[i]->getQuantity();
            }
        }


    if( totalResaleValue > 0 ) {
    
        mSellAllButton.setVisible( mSellMode );
    
        char *sellAllTip = autoSprintf( translate( "sellAllTip" ), 
                                        totalResaleValue );

        mSellAllButton.setMouseOverTip( sellAllTip );
        
        delete [] sellAllTip;
        }
    else {
        mSellAllButton.setVisible( false );
        }
    

    checkBuyButtonStatus();
    }



void LoadBackpackPage::undoActionOrActionGroup() {
    char stillInGroup = true;

    while( stillInGroup && mPurchaseHistory.size() > 0 ) {
            
        PurchaseHistoryRecord lastTransaction = 
            *( mPurchaseHistory.getElement( 
                   mPurchaseHistory.size() - 1 ) );
            
        
        mPurchaseHistory.deleteElement( mPurchaseHistory.size() - 1 );

        if( lastTransaction.quantity < 0 ) {
            // undoing a sale instead
            int id = lastTransaction.objectID;
            // might be multiple quantity sold at once
            int quantity = - lastTransaction.quantity;
                
            subtractFromQuantity( &mSellRecords, id, quantity );
                            
            mLootValue -= quantity * mToolPicker.getSellBackPrice( id );

            // stick undone sales back in pack or vault
            char found = false;
                
            if( lastTransaction.targetSlotType == 0 ) {    
                // pack slot already contains these items?
                for( int i=0; i<NUM_PACK_SLOTS; i++ ) {
                    if( mPackSlots[i]->getObject() == id ) {
                        mPackSlots[i]->addToQuantity( quantity );
                        found = true;
                        break;
                        }
                    }

                if( !found ) {
                    // stick in an empty pack slot
                    for( int i=0; i<NUM_PACK_SLOTS; i++ ) {
                        if( mPackSlots[i]->getObject() == -1 ) {
                            mPackSlots[i]->setObject( id );
                            mPackSlots[i]->setQuantity( quantity );
                            mPackSlots[i]->setSellPrice( 
                                mToolPicker.getSellBackPrice( id ) );
                            found = true;
                            break;
                            }
                        }
                    }
                }
                
            // no pack slots found to accommodate this undone sale
            // OR sale wasn't out of a pack slot in the first place

            if( !found ) {
                // vault slot already contains these items?
                for( int i=0; i<NUM_VAULT_SLOTS; i++ ) {
                    if( mVaultSlots[i]->getObject() == id ) {
                        mVaultSlots[i]->setQuantity( 
                            mVaultSlots[i]->getQuantity() + quantity );
                        found = true;
                        break;
                        }
                    }
                }
                
            if( ! found ) {
                // stick in an empty vault slot
                for( int i=0; i<NUM_VAULT_SLOTS; i++ ) {
                    if( mVaultSlots[i]->getObject() == -1 ) {
                        mVaultSlots[i]->setObject( id );
                        mVaultSlots[i]->setQuantity( quantity );
                        mVaultSlots[i]->setSellPrice( 
                            mToolPicker.getSellBackPrice( id ) );
                        found = true;
                        break;
                        }
                    }
                }       
            }
        else {
            // undoing a purchase (always made one at a time)
            int idToUnbuy = lastTransaction.objectID;
            subtractFromQuantity( &mPurchaseRecords, idToUnbuy );
            
            mLootValue += mToolPicker.getPrice( idToUnbuy );
                
                
            char found = false;
                
            if( lastTransaction.targetSlotType == 0 ) {
                // find in backpack?
                for( int i=NUM_PACK_SLOTS - 1; i>=0; i-- ) {
                    if( mPackSlots[i]->getObject() == idToUnbuy ) {
                        mPackSlots[i]->addToQuantity( -1 );
                        found = true;
                        break;
                        }
                    }
                }
            
            if( !found ) {
                // check vault
                for( int i=0; i<NUM_VAULT_SLOTS; i++ ) {
                    if( mVaultSlots[i]->getObject() == idToUnbuy ) {
                        mVaultSlots[i]->setQuantity( 
                            mVaultSlots[i]->getQuantity() - 1 );
                        found = true;
                        break;
                        }
                    }
                }
            }
        stillInGroup = lastTransaction.chainedAction;    
        }
    }

    
        
                


void LoadBackpackPage::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == &mDoneButton ) {
        mDone = true;
        }
    else if( inTarget == &mToolPicker ) {
        if( mToolPicker.shouldShowGridView() ) {
            mShowGridToolPicker = true;
            }
        else {
            checkBuyButtonStatus();
            }
        }
    else if( inTarget == &mUndoButton ) {
        undoActionOrActionGroup();
        
        checkBuyButtonStatus();
        
        checkSellModeStatus();
        
        checkUndoStatus();

        actionHappened();
        }
    else if( inTarget == &mBuyButton ) {
        mChangeHappened = true;
        
        int selectedObject = mToolPicker.getSelectedObject();
        
        int price = mToolPicker.getPrice( selectedObject );
        int sellPrice = mToolPicker.getSellBackPrice( selectedObject );;
        
        // first look for matching slot
        char found = false;
        
        for( int i=0; i<NUM_PACK_SLOTS; i++ ) {
            if( mPackSlots[i]->getObject() == selectedObject ) {
                found = true;
                
                mPackSlots[i]->addToQuantity( 1 );
                mLootValue -= price;
                
                mToolPicker.useSelectedObject();
                
                addToQuantity( &mPurchaseRecords, selectedObject );
                
                PurchaseHistoryRecord historyRecord = { selectedObject, 1, 0,
                                                        false };
                mPurchaseHistory.push_back( historyRecord );
                
                checkUndoStatus();

                break;
                }
            }
        
        if( ! found ) {
            // check for empty slot instead
            for( int i=0; i<NUM_PACK_SLOTS; i++ ) {
                if( mPackSlots[i]->getObject() == -1 ) {
                    // empty slot!
                    mPackSlots[i]->setObject( selectedObject );
                    mPackSlots[i]->setSellPrice( sellPrice );
                    mLootValue -= price;
                
                    mToolPicker.useSelectedObject();

                    addToQuantity( &mPurchaseRecords, selectedObject );

                    PurchaseHistoryRecord historyRecord = { selectedObject, 1,
                                                            0, false };
                    mPurchaseHistory.push_back( historyRecord );
                
                    checkUndoStatus();

                    break;
                    }
                }
            }
        
        checkBuyButtonStatus();
        actionHappened();
        }
    else if( inTarget == &mSellModeButton ) {
        mSellMode = true;
        checkSellModeStatus();
        }
    else if( inTarget == &mSellHalfButton ) {
        mSellHalfMode = true;
        checkSellModeStatus();
        }
    else if( inTarget == &mSellOneButton ) {
        mSellHalfMode = false;
        checkSellModeStatus();
        }
    else if( inTarget == &mSellAllButton ) {
        char firstRecordInChain = true;

        // process slots in reverse order so that when we undo back
        // through the chain, the original item order is restored
        for( int i=NUM_PACK_SLOTS - 1; i>=0; i-- ) {
            int id = mPackSlots[i]->getObject();
                
            if( id != -1 ) {
                int quantitySold = mPackSlots[i]->getQuantity();
                    
                mPackSlots[i]->setQuantity( 0 );
                    
                mLootValue += 
                    quantitySold * 
                    mToolPicker.getSellBackPrice( id );
                    
                addToQuantity( &mSellRecords, id, quantitySold );

                PurchaseHistoryRecord historyRecord = 
                    { id, -quantitySold, 0, ! firstRecordInChain };
                mPurchaseHistory.push_back( historyRecord );
                    
                firstRecordInChain = false;
                }
            }
        for( int i=NUM_VAULT_SLOTS - 1; i>=0; i-- ) {
            int id = mVaultSlots[i]->getObject();
                
            if( id != -1 ) {
                int quantitySold = mVaultSlots[i]->getQuantity();
                    
                mVaultSlots[i]->setQuantity( 0 );
                    
                mLootValue += 
                    quantitySold * 
                    mToolPicker.getSellBackPrice( id );
                    
                addToQuantity( &mSellRecords, id, quantitySold );

                PurchaseHistoryRecord historyRecord = 
                    { id, -quantitySold, 1, ! firstRecordInChain };
                mPurchaseHistory.push_back( historyRecord );
                    
                firstRecordInChain = false;
                }
            }

        checkSellModeStatus();
        
        checkUndoStatus();
        }
    else if( inTarget == &mBuyModeButton ) {
        mSellMode = false;
        mSellHalfMode = false;
        mMoveHalfMode = false;
        checkSellModeStatus();
        }
    else if( inTarget == &mMoveHalfButton ) {
        mMoveHalfMode = true;
        checkSellModeStatus();
        }
    else if( inTarget == &mMoveOneButton ) {
        mMoveHalfMode = false;
        checkSellModeStatus();
        }
    else {
        
        // check if backpack slot clicked
        char foundHit = false;
        
        for( int i=0; i<NUM_PACK_SLOTS; i++ ) {
            if( inTarget == mPackSlots[i] ) {
                int id = mPackSlots[i]->getObject();
                
                if( id != -1 ) {
                    // click on a non-empty pack slot counts as an action
                    mChangeHappened = true;

                    actionHappened();
                    
                    
                    
                    if( mSellMode ) {

                        int quantitySold = 1;
                            
                        if( mSellHalfMode ) {
                            quantitySold = mPackSlots[i]->getQuantity() / 2;
                            }
                        if( quantitySold < 1 ) {
                            quantitySold = 1;
                            }
                        mPackSlots[i]->addToQuantity( - quantitySold );

                        mLootValue += 
                            quantitySold * 
                            mToolPicker.getSellBackPrice( id );
                        
                        addToQuantity( &mSellRecords, id, quantitySold );
                        PurchaseHistoryRecord historyRecord = 
                            { id, -quantitySold, 0, false };
                        mPurchaseHistory.push_back( historyRecord );

                        checkSellModeStatus();
                        checkUndoStatus();
                        }
                    else {
                        int quantityMoved = 1;
                        if( mMoveHalfMode ) {
                            quantityMoved = mPackSlots[i]->getQuantity() / 2;
                            }
                        if( quantityMoved < 1 ) {
                            quantityMoved = 1;
                            }
                        
                        mPackSlots[i]->addToQuantity( - quantityMoved );

                        char foundInVault = false;

                        for( int j=0; j<NUM_VAULT_SLOTS; j++ ) {
                            if( mVaultSlots[j]->getObject() == id ) {
                                mVaultSlots[j]->addToQuantity( quantityMoved );
                                foundInVault = true;
                                break;
                                }
                            }
                        if( !foundInVault ) {
                            // find empty slot and stick it there
                            for( int j=0; j<NUM_VAULT_SLOTS; j++ ) {
                                if( mVaultSlots[j]->getObject() == -1 ) {
                                    mVaultSlots[j]->setObject( id );
                                    mVaultSlots[j]->setQuantity( 
                                        quantityMoved );
                                    mVaultSlots[j]->setSellPrice(
                                        mToolPicker.getSellBackPrice( id ) );
                                    break;
                                    }
                                }
                            }
                        }
                    
                    checkBuyButtonStatus();
                    }
                
                foundHit = true;
                break;
                }
            }
        
        if( !foundHit ) {
            // check for clicks of vault slots

            for( int i=0; i<NUM_VAULT_SLOTS; i++ ) {
                if( inTarget == mVaultSlots[i] ) {
                    int id = mVaultSlots[i]->getObject();
                
                    if( id != -1 ) {
                        // click on a non-empty vault slot counts as an action
                        mChangeHappened = true;

                        actionHappened();
                        
                        if( mSellMode ) {
                            int quantitySold = 1;
                            
                            if( mSellHalfMode ) {
                                quantitySold = 
                                    mVaultSlots[i]->getQuantity() / 2;
                                }
                            if( quantitySold < 1 ) {
                                quantitySold = 1;
                                }
                            

                            mVaultSlots[i]->setQuantity( 
                                mVaultSlots[i]->getQuantity()
                                - quantitySold );
                            
                            mLootValue += 
                                quantitySold * 
                                mToolPicker.getSellBackPrice( id );
                        
                            addToQuantity( &mSellRecords, id, quantitySold );
                            PurchaseHistoryRecord historyRecord = 
                                { id, -quantitySold, 1, false };
                            mPurchaseHistory.push_back( historyRecord );
                        
                            checkSellModeStatus();
                            checkUndoStatus();
                            }
                        else {
                            // ignore transfer click if backpack full
                            // and if it has no matching slot
                            int foundIndex = -1;
                            char empty = false;
                            for( int j=0; j<NUM_PACK_SLOTS; j++ ) {
                                if( mPackSlots[j]->getObject() == id ) {
                                    foundIndex = j;
                                    break;
                                    }
                                }
                            if( foundIndex == -1 ) {    
                                for( int j=0; j<NUM_PACK_SLOTS; j++ ) {
                                    if( mPackSlots[j]->getObject() == -1 ) {
                                        foundIndex = j;
                                        empty = true;
                                        break;
                                        }
                                    }
                                }
                        
                            if( foundIndex != -1 ) {

                                int quantityMoved = 1;
                                if( mMoveHalfMode ) {
                                    quantityMoved = 
                                        mVaultSlots[i]->getQuantity() / 2;
                                    }
                                if( quantityMoved < 1 ) {
                                    quantityMoved = 1;
                                    }

                                mVaultSlots[i]->addToQuantity( 
                                    - quantityMoved );
                                                                
                                if( empty ) {
                                    mPackSlots[foundIndex]->setObject( id );
                                    mPackSlots[foundIndex]->setQuantity( 
                                        quantityMoved );
                                    
                                    mPackSlots[foundIndex]->setSellPrice(
                                        mToolPicker.getSellBackPrice( id ) );
                                    }
                                else {
                                    mPackSlots[foundIndex]->addToQuantity( 
                                        quantityMoved );
                                    }
                                
                                checkBuyButtonStatus();
                                }
                            }
                        }
                    break;
                    }
                }
            }

        
        }
    
    
    }





        
void LoadBackpackPage::makeActive( char inFresh ) {
    LiveHousePage::makeActive( inFresh );
    
    if( !inFresh ) {
        return;
        }
    
    mDone = false;
    mShowGridToolPicker = false;
    mSellMode = false;
    mSellHalfMode = false;
    mMoveHalfMode = false;
    
    checkSellModeStatus();
    }



void LoadBackpackPage::drawUnderComponents( doublePair inViewCenter, 
                                            double inViewSize ) {
    
    drawBalance( mLootValue, 0 );
    }



void LoadBackpackPage::draw( doublePair inViewCenter, 
                             double inViewSize ) {
        
    doublePair labelPos = { 0, 6.75 };

    if( mSellMode ) {    
        drawMessage( "loadBackpackSellModeDescription", labelPos, false );
        }    
    else {
        drawMessage( "loadBackpackDescription", labelPos, false );
        }
    


    setDrawColor( 1, 1, 1, 1 );

    labelPos = mPackSlots[0]->getCenter();
    
    labelPos.y += 1;
    labelPos.x -= 1;

    mainFont->drawString( translate( "backpackLabel" ), labelPos, alignLeft );
    
    
    labelPos = mVaultSlots[0]->getCenter();
    
    labelPos.y += 1;
    labelPos.x -= 1;

    mainFont->drawString( translate( "vaultLabel" ), labelPos, alignLeft );
    



    }
