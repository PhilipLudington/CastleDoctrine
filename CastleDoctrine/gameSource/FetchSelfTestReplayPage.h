#ifndef FETCH_SELF_TEST_REPLAY_PAGE_INCLUDED
#define FETCH_SELF_TEST_REPLAY_PAGE_INCLUDED


#include "GamePage.h"
#include "FetchRobberyReplayPage.h"

#include "TextField.h"
#include "TextButton.h"


#include "minorGems/ui/event/ActionListener.h"



class FetchSelfTestReplayPage : public GamePage, public ActionListener {
        
    public:
        
        FetchSelfTestReplayPage();
        
        virtual ~FetchSelfTestReplayPage();

        // must be called before makeActive
        void setOwnerID( int inID );
        void setOwnerCharacterName( const char *inName );

        virtual char getReturnToMenu();
        
        virtual char getRecordReady();

        // members destroyed internally
        virtual RobberyLog getLogRecord();
        
        virtual void actionPerformed( GUIComponent *inTarget );


        virtual void step();
        
        virtual void makeActive( char inFresh );

    protected:
        
        int mWebRequest;

        char mRecordReady;
        RobberyLog mLogRecord;

        TextButton mMenuButton;


        char mReturnToMenu;
        
        int mOwnerID;
        char *mOwnerCharacterName;
        
        void clearRecord();
    };



#endif
