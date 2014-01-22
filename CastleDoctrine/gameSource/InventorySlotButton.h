#ifndef INVENTORY_SLOT_BUTTON_INCLUDED
#define INVENTORY_SLOT_BUTTON_INCLUDED


#include "SpriteButton.h"


#include "minorGems/game/Font.h"



class InventorySlotButton : public SpriteButton {
        

    public:

        InventorySlotButton( Font *inDisplayFont,
                             double inX, double inY, 
                             double inDrawScale = 1.0 );

        // Controls tool tip
        // 0 for no transfer on click
        // 1 for transfer one item to vault
        // 2 for transfer one item to backpack
        // 3 for selling one item
        // 4 for selling half of items
        // 5 for transfer half of items to vault
        // 6 for transfer half items to backpack
        //
        // defaults to 0
        virtual void setTransferStatus( int inStatus );
        

        // if ID is -1, slot is cleared
        virtual void setObject( int inID );

        // set to 0 clears object ID
        void setQuantity( int inQuantity );
        
        int getQuantity();

        // inDelta can be positive or negative
        // if it brings quantity to zero, object ID is cleared
        void addToQuantity( int inDelta );
        

        // used for tool tip when transfer status is 3 (sell mode)
        void setSellPrice( int inSellPrice );
        

        // -1 if empty
        int getObject();
        

        void setRingOn( char inRingOn );
        
        char getRingOn() {
            return mRingOn;
            }
        

    protected:
        Font *mFont;

        int mObjectID;
        
        int mQuantity;
        
        
        int mTransferStatus;
        
        int mSellPrice;
        

        // override from SpriteButton to display quantity
        virtual void drawContents();

        char mRingOn;
        
        // override this from Button to draw ring
        virtual void draw();


        void setHalfSellTip();

        // true to generate tip for vault slot, false for backpack slot
        void setHalfMoveTip( char inVault );
        
    };



#endif
