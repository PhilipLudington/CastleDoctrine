#include "LiveHousePage.h"

#include "TextButton.h"
#include "KeyEquivalentTextButton.h"
#include "InventorySlotButton.h"

#include "HouseObjectPicker.h"

#include "inventory.h"



#include "minorGems/ui/event/ActionListener.h"
#include "minorGems/util/SimpleVector.h"



typedef struct PurchaseHistoryRecord {
        int objectID;
        int quantity;
        
        // 0 for pack
        // 1 for vault
        int targetSlotType;


        // set to true if this purchase/sale was part of a bulk action
        // (a series of purchases/sales that all happened together as one
        //  action, like "sell all")
        // They should be undone together as one action
        // If true, this action is grouped with the action before it
        // (and maybe the action before that one, if the chain continues).
        char chainedAction;

    } PurchaseHistoryRecord;




class LoadBackpackPage : public LiveHousePage, public ActionListener {
        
    public:
        
        LoadBackpackPage();
        
        virtual ~LoadBackpackPage();

        // destroyed by caller
        void setVaultContents( char *inVaultContents );
        char *getVaultContents();


        // destroyed by caller
        void setBackpackContents( char *inBackpackContents );
        char *getBackpackContents();
        
        // list of purchases added to vault/backpack
        void setPurchaseList( char *inPurchaseList );
        char *getPurchaseList();
        
        // list of sold items removed from vault/backpack
        void setSellList( char *inSellList );
        char *getSellList();

        
        // destroyed by caller
        void setPriceList( char *inPriceList );
        
        
        void setLootValue( int inLootValue );
        
        int getLootValue() {
            return mLootValue;
            }
        

        // true if contents of backpack or vault touched in any way
        char getChangeHappened() {
            return mChangeHappened;
            }
        

        char getDone() {
            return mDone;
            }
        

        char showGridToolPicker() {
            return mShowGridToolPicker;
            }
        
        HouseObjectPicker *getToolPicker() {
            return &mToolPicker;
            }


        virtual void actionPerformed( GUIComponent *inTarget );


        virtual void draw( doublePair inViewCenter, 
                           double inViewSize );
        
        
        virtual void makeActive( char inFresh );



    protected:
        
        int mLootValue;
        
        char mSellMode;
        char mSellHalfMode;

        char mMoveHalfMode;

        HouseObjectPicker mToolPicker;
        TextButton mDoneButton;
        TextButton mSellModeButton;
        TextButton mSellHalfButton;
        TextButton mSellOneButton;
        TextButton mSellAllButton;

        TextButton mBuyModeButton;

        TextButton mMoveHalfButton;
        TextButton mMoveOneButton;

        KeyEquivalentTextButton mUndoButton;
        
        SpriteButton mBuyButton;

        char mDone;
        char mShowGridToolPicker;

        char mChangeHappened;
        

        InventorySlotButton *mPackSlots[ NUM_PACK_SLOTS ];

        InventorySlotButton *mVaultSlots[ NUM_VAULT_SLOTS ];

        SimpleVector<QuantityRecord> mPurchaseRecords;
        SimpleVector<QuantityRecord> mSellRecords;
        
        // for undo
        // positive quanties here represent purchases
        // negative quantities represent sales
        SimpleVector<PurchaseHistoryRecord> mPurchaseHistory;
        


        void undoActionOrActionGroup();
        
        void checkBuyButtonStatus();
        void checkUndoStatus();
        
        void checkSellModeStatus();


    };

