#include "GamePage.h"

#include "TextField.h"
#include "TextButton.h"
#include "inventory.h"
#include "GalleryArchive.h"

#include "minorGems/ui/event/ActionListener.h"


class RobCheckinHousePage : public GamePage, public ActionListener {
        
    public:
        
        RobCheckinHousePage();
        
        virtual ~RobCheckinHousePage();


        virtual char getReturnToHome();
        
        // did robber die?
        virtual char getStartOver();
        

        // destroyed by caller
        virtual void setHouseMap( char *inHouseMap );
        
        virtual void setSuccess( int inSuccess );

        // destoryed by caller
        virtual void setBackpackContents( char *inBackpackContents );

        // destoryed by caller
        virtual void setMoveList( char *inMoveList );

        virtual void setWifeKilledRobber( char inKilledRobber );
        
        virtual void setWifeKilled( char inKilled );
        virtual void setWifeRobbed( char inRobbed );
        virtual void setFamilyKilledCount( int inNumKilled );

        virtual void actionPerformed( GUIComponent *inTarget );
        


        virtual void step();
        
        virtual void draw( doublePair inViewCenter, 
                           double inViewSize );
        
        virtual void makeActive( char inFresh );


    protected:

        int mWebRequest;

        char *mBackpackContents;
        char *mMoveList;
        char *mHouseMap;
        
        char mWifeKilledRobber;

        char mWifeKilled;
        char mWifeRobbed;
        int mFamilyKilledCount;
        
        int mSuccess;
        
        int mMoneyTaken;


        TextButton mHomeButton;
        TextButton mStartOverButton;
        
        InventorySlotButton *mVaultSlots[ NUM_VAULT_SLOTS ];

        GalleryArchive mGalleryDisplay;
        
        char mReturnToHome;
        char mStartOver;

    };

