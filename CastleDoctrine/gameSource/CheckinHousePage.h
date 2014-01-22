#include "LiveHousePage.h"

#include "TextField.h"
#include "TextButton.h"

#include "minorGems/ui/event/ActionListener.h"


class CheckinHousePage : public LiveHousePage, public ActionListener {
        
    public:
        
        CheckinHousePage();
        
        virtual ~CheckinHousePage();


        virtual char getReturnToMenu();
        virtual char getStartOver();
        
        // destroyed by caller
        virtual void setHouseMap( const char *inHouseMap );        
        virtual void setVaultContents( const char *inVaultContents );
        virtual void setBackpackContents( const char *inBackpackContents );
        virtual void setGalleryContents( const char *inGalleryContents );
        virtual void setFamilyExitPaths( const char *inFamilyExitPaths );
        virtual void setPurchaseList( const char *inPurchaseList );
        virtual void setSellList( const char *inSellList );
        virtual void setPriceList( const char *inPriceList );
        virtual void setMoveList( const char *inMoveList );
        
        // 0 or 1
        virtual void setDied( int inDied );
        

        virtual void actionPerformed( GUIComponent *inTarget );
        


        virtual void step();

        
        virtual void makeActive( char inFresh );


    protected:
        int mRequestStarted;
        
        int mWebRequest;

        char *mHouseMap;
        char *mVaultContents;
        char *mBackpackContents;
        char *mGalleryContents;
        char *mFamilyExitPaths;
        char *mPurchaseList;
        char *mSellList;
        char *mPriceList;
        char *mMoveList;
        
        int mDied;
        

        TextButton mMenuButton;
        TextButton mStartOverButton;


        char mReturnToMenu;
        char mStartOver;

    };

