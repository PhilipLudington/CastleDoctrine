#include "LiveHousePage.h"

#include "TextField.h"
#include "TextButton.h"
#include "KeyEquivalentTextButton.h"
#include "SpriteToggleButton.h"
#include "CheckboxButton.h"

#include "EyedropperStatusDisplay.h"


#include "HouseGridDisplay.h"
#include "HouseObjectPicker.h"

#include "Gallery.h"

#include "minorGems/ui/event/ActionListener.h"




class EditHousePage : public LiveHousePage, public ActionListener {
        
    public:
        
        EditHousePage();
        
        virtual ~EditHousePage();
        

        // destroyed by caller
        void setWifeName( const char *inWifeName );
        void setSonName( const char *inSonName );
        void setDaughterName( const char *inDaughterName );
        
        char *getWifeName();
        char *getSonName();
        char *getDaughterName();
        
        
        // destroyed by caller
        void setHouseMap( const char *inHouseMap );
        char *getHouseMap();


        // destroyed by caller
        void setVaultContents( const char *inVaultContents );
        char *getVaultContents();


        // destroyed by caller
        void setBackpackContents( const char *inBackpackContents );
        char *getBackpackContents();

        // destroyed by caller
        void setGalleryContents( const char *inGalleryContents );
        char *getGalleryContents();


        // Must be called before setLootValue
        void setNumberOfTapes( int inNumber );
        

        // destroyed by caller
        char *getEditList();
        char *getFamilyExitPaths();
        
        char getWifeLiving();
        

        // list of purchases added to vault/backpack
        void setPurchaseList( const char *inPurchaseList );
        char *getPurchaseList();

        void setSellList( const char *inSellList );
        char *getSellList();
        
        
        // destroyed by caller
        void setPriceList( const char *inPriceList );
        char *getPriceList();
        
        
        void setLootValue( int inLootValue );
        
        int getLootValue() {
            return mLootValue;
            }
        
        void setMustSelfTest( char inMustSelfTest );
        

        void setBackpackOrVaultChanged() {
            mBackpackOrVaultChanged = true;
            }


        // true if map changed since last setHouseMap call
        // or if self-map-test required for any other reason
        char houseMapChanged();
        

        char shouldJumpToTapes() {
            return mJumpToTapes;
            }
        

        char getDone() {
            return mDone;
            }
        char getDead() {
            return mDead;
            }

        char showLoadBackpack() {
            return mShowLoadBackpack;
            }
        
        char showAuctions() {
            return mShowAuctions;
            }

        char showGridObjectPicker() {
            return mShowGridObjectPicker;
            }
        
        HouseObjectPicker *getObjectPicker() {
            return &mObjectPicker;
            }
            

        virtual void actionPerformed( GUIComponent *inTarget );


        virtual void drawUnderComponents( doublePair inViewCenter, 
                                          double inViewSize );

        virtual void draw( doublePair inViewCenter, 
                           double inViewSize );
        
        
        virtual void makeActive( char inFresh );


        // override from LiveHousePage to block jumping to tapes
        // when there are LiveHousePage requests pending
        virtual void step();


        // override to handle print-whole-map key
        virtual void keyDown( unsigned char inASCII );



    protected:

        // for change detection
        char *mStartHouseMap;
        
        char *mVaultContents;
        char *mBackpackContents;
        
        char *mPriceList;
        
        char *mPurchaseList;
        char *mSellList;

        int mLootValue;

        int mChangesCost;

        char mMustSelfTest;

        char mMapStartedOutEmpty;
        
        char mBackpackOrVaultChanged;

        HouseObjectPicker mObjectPicker;
        HouseGridDisplay mGridDisplay;
        TextButton mDoneButton;
        TextButton mBackpackButton;
        TextButton mAuctionButton;
        KeyEquivalentTextButton mUndoButton;
        TextButton mJumpToTapesButton;
        TextButton mSuicideButton;
        CheckboxButton mSuicideConfirmCheckbox;

        SpriteToggleButton mDiffHighlightToggleButton;
        
        EyedropperStatusDisplay mEyedropperStatus;
        

        // Once undo pressed, never show suicide button again
        // (it occupies same spot, and too risky for user to hit
        //  undo-undo-undo-suicide)
        // If they are able to edit house, suicide should not be an option
        // anyway.
        char mBlockSuicideButton;
        

        Gallery mGallery;
        
        int mNumberOfTapes;
        char mJumpToTapes;
        char mDone;
        char mDead;
        char mShowLoadBackpack;
        char mShowAuctions;
        char mShowGridObjectPicker;
        
        void checkIfPlacementAllowed();

        void checkIfDoneButtonVisible();
      
        void checkIfTapesButtonVisible();

        void recomputeChangeCost();
        
        
        // override because large icon overlaps with picker
        virtual char makeWaitingIconSmall() {
            return true;
            }

    };

