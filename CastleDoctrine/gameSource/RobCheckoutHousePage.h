#include "GamePage.h"

#include "TextField.h"
#include "TextButton.h"


#include "minorGems/ui/event/ActionListener.h"


class RobCheckoutHousePage : public GamePage, public ActionListener {
        
    public:
        
        RobCheckoutHousePage();
        
        virtual ~RobCheckoutHousePage();

        // must be called before makeActive
        void setToRobHomeID( int inID );
        void setToRobCharacterName( const char *inName );

        virtual char getReturnToMenu();
        

        // destroyed by caller if not NULL
        virtual char *getWifeName();
        virtual char *getSonName();
        virtual char *getDaughterName();


        // destroyed by caller if not NULL
        virtual char *getOwnerName();

        // destroyed by caller if not NULL
        // can only be called once per checkout (because this
        // call destroys/clears the internal house map)
        virtual char *getHouseMap();
        
        // destroyed by caller if not NULL
        virtual char *getBackpackContents();

        // destroyed by caller if not NULL
        virtual char *getGalleryContents();

        virtual int getWifeMoney();

        virtual int getMusicSeed();

        virtual int getMaxSeconds();

        virtual void actionPerformed( GUIComponent *inTarget );


        virtual void step();
        
        virtual void makeActive( char inFresh );

    protected:
        void clearDataMembers();
        
        int mWebRequest;
        char *mMapEncryptionKey;
        
        char *mWifeName;
        char *mSonName;
        char *mDaughterName;
        char *mOwnerName;
        char *mHouseMap;
        char *mBackpackContents;
        char *mGalleryContents;
        int mWifeMoney;
        int mMusicSeed;
        int mMaxSeconds;

        TextButton mMenuButton;


        char mReturnToMenu;
        
        int mToRobHomeID;
        char *mToRobCharacterName;
    };

