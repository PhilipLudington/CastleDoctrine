#include "InventorySlotButton.h"


#include "tools.h"


#include "minorGems/util/stringUtils.h"

#include "minorGems/game/drawUtils.h"
#include "minorGems/game/gameGraphics.h"
#include "minorGems/game/game.h"



InventorySlotButton::InventorySlotButton( Font *inDisplayFont,
                                          double inX, double inY,
                                          double inDrawScale )
        :
        // lie about sprite size so that button padding isn't so huge 
        SpriteButton( NULL, 18, 18, inX, inY, inDrawScale ),
        mFont( inDisplayFont ),
        mObjectID( -1 ),
        mQuantity( 0 ),
        mTransferStatus( 0 ),
        mSellPrice( 0 ),
        mRingOn( false ) {
    
    mPixWidth *= 2;
    
    mOverrideHighlightColor = true;
    }



void InventorySlotButton::setTransferStatus( int inStatus ) {
    mTransferStatus = inStatus;
    
    // reset object (potentially change to tool tip)
    int oldQuantity = mQuantity;
    setObject( mObjectID );
    setQuantity( oldQuantity );
    }



void InventorySlotButton::setSellPrice( int inSellPrice ) {
    mSellPrice = inSellPrice;

    // reset object (potentially change to tool tip)
    int oldQuantity = mQuantity;
    setObject( mObjectID );
    setQuantity( oldQuantity );
    }



void InventorySlotButton::setHalfSellTip() {
    char *tip;
    

    if( mQuantity > 3 ) {
        int sellQuantity = mQuantity / 2;
        const char *toolDescription = getToolDescriptionPlural( mObjectID );

        tip = autoSprintf( translate( "sellHalfSlotTip" ),
                           sellQuantity, toolDescription, 
                           sellQuantity * mSellPrice );
        }
    else {
        const char *toolDescription = getToolDescription( mObjectID );

        tip = autoSprintf( translate( "sellSlotTip" ),
                           toolDescription, mSellPrice );
        }
    
    setMouseOverTip( tip );
    delete [] tip;
    }




void InventorySlotButton::setHalfMoveTip( char inVault ) {

    const char *tipHalf = "backpackHalfSlotTip";
    const char *tipOne = "backpackSlotTip";
    
    if( inVault ) {
        tipHalf = "vaultHalfSlotTip";
        tipOne = "vaultSlotTip";
        }
    
    char *tip;
    

    if( mQuantity > 3 ) {
        int sellQuantity = mQuantity / 2;
        const char *toolDescription = getToolDescriptionPlural( mObjectID );

        tip = autoSprintf( translate( tipHalf ),
                           sellQuantity, toolDescription, 
                           sellQuantity * mSellPrice );
        }
    else {
        const char *toolDescription = getToolDescription( mObjectID );

        tip = autoSprintf( translate( tipOne ),
                           toolDescription, mSellPrice );
        }
    
    setMouseOverTip( tip );
    delete [] tip;
    }



void InventorySlotButton::setObject( int inID ) {
    mObjectID = inID;
    
    if( mObjectID == -1 ) {
        setMouseOverTip( "" );
        mSprite = NULL;
        mQuantity = 0;
        }
    else {
        mQuantity = 1;

        const char *toolDescription = getToolDescription( mObjectID );
        
        switch( mTransferStatus ) {
            case 1: {
                char *tip = autoSprintf( translate( "backpackSlotTip" ),
                                         toolDescription );
                setMouseOverTip( tip );
                delete [] tip;
                }
                break;
            case 2: {
                char *tip = autoSprintf( translate( "vaultSlotTip" ),
                                         toolDescription );
                setMouseOverTip( tip );
                delete [] tip;
                }
                break;
            case 3: {
                char *tip = autoSprintf( translate( "sellSlotTip" ),
                                         toolDescription, mSellPrice );
                setMouseOverTip( tip );
                delete [] tip;
                }
                break;
            case 4: {
                setHalfSellTip();
                }
                break;
            case 5: {
                setHalfMoveTip( false );
                }
                break;
            case 6: {
                setHalfMoveTip( true );
                }
                break;
            default:
                setMouseOverTip( toolDescription );
                break;
            }
                
        mSprite = getToolSprite( mObjectID );
        }
    }


        
int InventorySlotButton::getObject() {
    return mObjectID;
    }



void InventorySlotButton::setQuantity( int inQuantity ) {
    mQuantity = inQuantity;
    
    if( mQuantity == 0 ) {
        setObject( -1 );
        }
    else {
        switch( mTransferStatus ) {
            case 4:
                setHalfSellTip();
                break;
            case 5:
                setHalfMoveTip( false );
                break;
            case 6:
                setHalfMoveTip( true );
                break;
            }
        }
    }


        
int InventorySlotButton::getQuantity() {
    return mQuantity;
    }



void InventorySlotButton::addToQuantity( int inDelta ) {
    setQuantity( getQuantity() + inDelta );
    }




void InventorySlotButton::drawContents() {
    SpriteButton::drawContents();
    
    if( mQuantity > 1 ) {
        
        
        // darken corner

        double verts[6];
        float colors[12];
        
        verts[0] = -mWide / 2;
        verts[1] = -mHigh / 2;

        colors[0] = 0;
        colors[1] = 0;
        colors[2] = 0;
        colors[3] = 0;
        
        
        verts[2] = mWide / 2;
        verts[3] = -mHigh / 2;

        colors[4] = 0;
        colors[5] = 0;
        colors[6] = 0;
        colors[7] = 0.75;


        verts[4] = mWide / 2;
        verts[5] = mHigh / 2;

        
        colors[8] = 0;
        colors[9] = 0;
        colors[10] = 0;
        colors[11] = 0;

        drawTrianglesColor( 1, verts, colors );
        


        doublePair textPos = { 0.5 - mDrawScale, -0.5 };
        
        char *text = autoSprintf( "%d", mQuantity );
        
        setDrawColor( 0.828, 0.647, 0.212, 1 );

        mFont->drawString( text, textPos, alignRight );
    
        delete [] text;
        }
    
    }




void InventorySlotButton::setRingOn( char inRingOn ) {
    mRingOn = inRingOn;
    }





void InventorySlotButton::draw() {

    if( mRingOn ) {

        // ring
        setDrawColor( 0.25, 1.0, 0.93, 1 );
        
        double fullWide = mWide + 4 * mPixWidth;
        double fullHigh = mHigh + 4 * mPixWidth;

        drawRect( - fullWide / 2, - fullHigh / 2, 
                  fullWide / 2, fullHigh / 2 );


        // background inside ring
        setDrawColor( 0, 0, 0, 1 );
        
        fullWide -= 2 * mPixWidth;
        fullHigh -= 2 * mPixWidth;
        
        drawRect( - fullWide / 2, - fullHigh / 2, 
                  fullWide / 2, fullHigh / 2 );
        }
    

    // draw button's contents on top
    SpriteButton::draw();
    }
