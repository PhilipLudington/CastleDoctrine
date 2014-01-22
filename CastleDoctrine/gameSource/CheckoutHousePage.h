#include "GamePage.h"

#include "TextField.h"
#include "TextButton.h"


#include "minorGems/ui/event/ActionListener.h"


class CheckoutHousePage : public GamePage, public ActionListener {
        
    public:
        
        CheckoutHousePage();
        
        virtual ~CheckoutHousePage();


        virtual char getReturnToMenu();
        
        
        // destroyed by caller if not NULL
        virtual char *getWifeName();
        virtual char *getSonName();
        virtual char *getDaughterName();

        virtual int getPaymentCount();
        virtual int getYouPaidTotal();
        virtual int getWifePaidTotal();

        virtual int getNumberOfTapes();
        
        
        
        // destroyed by caller if not NULL
        virtual char *getHouseMap();

        // destroyed by caller if not NULL
        virtual char *getVaultContents();

        // destroyed by caller if not NULL
        virtual char *getBackpackContents();

        // destroyed by caller if not NULL
        virtual char *getGalleryContents();

        // destroyed by caller if not NULL
        virtual char *getPriceList();

        virtual int getLootValue();
        
        virtual char getMustSelfTest();
        
        virtual int getMusicSeed();
        
        virtual void actionPerformed( GUIComponent *inTarget );


        virtual void step();
        
        virtual void makeActive( char inFresh );

    protected:

        int mWebRequest;

        char *mWifeName;
        char *mSonName;
        char *mDaughterName;
        
        int mPaymentCount;
        int mYouPaidTotal;
        int mWifePaidTotal;
        int mNumberOfTapes;

        char *mHouseMap;
        char *mVaultContents;
        char *mBackpackContents;
        char *mGalleryContents;
        char *mPriceList;
        
        int mLootValue;

        char mMustSelfTest;
        
        int mMusicSeed;

        TextButton mMenuButton;


        char mReturnToMenu;

    };

