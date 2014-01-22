#include "GamePage.h"

#include "TextField.h"
#include "TextButton.h"


#include "minorGems/ui/event/ActionListener.h"


class FetchBlueprintPage : public GamePage, public ActionListener {
        
    public:
        
        FetchBlueprintPage();
        
        virtual ~FetchBlueprintPage();

        // must be called before makeActive
        void setToRobUserID( int inID );
        void setToRobCharacterName( const char *inName );

        virtual char getReturnToMenu();
        



        // destroyed by caller if not NULL
        virtual char *getOwnerName();

        // destroyed by caller if not NULL
        virtual char *getHouseMap();

        virtual void actionPerformed( GUIComponent *inTarget );


        virtual void step();
        
        virtual void makeActive( char inFresh );

    protected:
        void clearDataMembers();
        
        int mWebRequest;
        
        char *mOwnerName;
        char *mHouseMap;
        
        TextButton mMenuButton;


        char mReturnToMenu;
        
        int mToRobUserID;
        char *mToRobCharacterName;
    };

