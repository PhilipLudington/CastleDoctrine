#include "GamePage.h"

#include "TextButton.h"
#include "SpriteToggleButton.h"

#include "ReplayRobHouseGridDisplay.h"
#include "FetchRobberyReplayPage.h"

#include "inventory.h"


#include "minorGems/ui/event/ActionListener.h"



class ReplayRobHousePage : public GamePage, public ActionListener {
        
    public:
        
        ReplayRobHousePage();
        
        virtual ~ReplayRobHousePage();
        
        
        // members destroyed by caller
        void setLog( RobberyLog inLog );
        
        
        char getDone() {
            return mDone;
            }
            

        virtual void actionPerformed( GUIComponent *inTarget );

        

        virtual void draw( doublePair inViewCenter, 
                           double inViewSize );
        
        
        virtual void makeActive( char inFresh );


        // override to handle print-whole-map key
        virtual void keyDown( unsigned char inASCII );


    protected:
        
        int mMusicSeed;

        ReplayRobHouseGridDisplay mGridDisplay;
        TextButton mDoneButton;
        SpriteToggleButton mMusicToggleButton;

        char *mPackSlotsString;
        
        InventorySlotButton *mPackSlots[ NUM_PACK_SLOTS ];

        
        char mDone;
        
        char *mDescription;

    };

