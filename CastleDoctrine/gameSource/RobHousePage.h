#include "LiveHousePage.h"

#include "TextField.h"
#include "TextButton.h"

#include "SpriteToggleButton.h"
#include "CheckboxButton.h"


#include "RobHouseGridDisplay.h"

#include "Gallery.h"

#include "inventory.h"



#include "minorGems/ui/event/ActionListener.h"


#include <time.h>




class RobHousePage : public LiveHousePage, public ActionListener {
        
    public:
        
        RobHousePage();
        
        virtual ~RobHousePage();
        

        // set to false to hide backpack buttons
        // defaults to showing them
        void showBackpack( char inShow );
        

        // destroyed by caller
        void setWifeName( const char *inWifeName );
        void setSonName( const char *inSonName );
        void setDaughterName( const char *inDaughterName );
        
        // destroyed by caller
        void setHouseMap( char *inHouseMap );
        char *getHouseMap();

        // gets starting map (not map as modified by robber so far)
        // destroyed by caller
        char *getBlueprintMap();
        

        virtual int getVisibleOffsetX();
        virtual int getVisibleOffsetY();


        // destroyed by caller
        void setBackpackContents( char *inBackpackContents );
        char *getBackpackContents();

        void setGalleryContents( char *inGalleryContents );
        
        void setWifeMoney( int inMoney );

        void setMusicSeed( int inMusicSeed );
        
        void setMaxSeconds( int inMaxSeconds );

        char getSuccess() {
            return mGridDisplay.getSuccess();
            }

        char *getMoveList() {
            return mGridDisplay.getMoveList();
            }


        char getWifeKilledRobber() {
            return mGridDisplay.getWifeKilledRobber();
            }
        

        char getWifeKilled() {
            return mGridDisplay.getWifeKilled();
            }

        char getWifeRobbed() {
            return mGridDisplay.getWifeRobbed();
            }
        
        int getFamilyKilledCount() {
            return mGridDisplay.getFamilyKilledCount();
            }


        char getViewBlueprint() {
            return mViewBlueprint;
            }


        char getDone() {
            return mDone;
            }
            


        void setDescription( const char *inDescription );
        


        virtual void actionPerformed( GUIComponent *inTarget );


        virtual void step();
        
        virtual void draw( doublePair inViewCenter, 
                   double inViewSize );

        
        virtual void makeActive( char inFresh );

    protected:
        char mShowBackpack;

        RobHouseGridDisplay mGridDisplay;
        TextButton mBlueprintButton;
        TextButton mDoneButton;
        CheckboxButton mSuicideConfirmCheckbox;

        SpriteToggleButton mMusicToggleButton;
        SpriteToggleButton mSafeMoveToggleButton;
        

        Gallery mGallery;
        
        int mMusicSeed;

        char mShowTimeAtEnd;
        time_t mEndTime;
        float mTimeMessageFade;
        float mTimeMessageFadeDirection;
        

        InventorySlotButton *mPackSlots[ NUM_PACK_SLOTS ];
        
        char mViewBlueprint;
        char mDone;

        char *mDescription;
        
        char *mDeathMessage;

        
        // override from GamePage to selectively hide any waiting
        // icons (if robTime message shown, because it overlaps with waiting
        // icon)
        virtual char canShowWaitingIcon();
        

        
    };

