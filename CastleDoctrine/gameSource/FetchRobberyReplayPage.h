#ifndef FETCH_ROBBERY_REPLAY_PAGE_INCLUDED
#define FETCH_ROBBERY_REPLAY_PAGE_INCLUDED


#include "GamePage.h"

#include "TextField.h"
#include "TextButton.h"


#include "minorGems/ui/event/ActionListener.h"


typedef struct RobberyLog {
        char *robberName;
        char *victimName;
        char *wifeName;
        char *sonName;
        char *daughterName;
        char *houseMap;
        char *backpackContents;
        char *moveList;
        int lootValue;
        int wifeMoney;
        int musicSeed;
        char isBounty;
    } RobberyLog;



class FetchRobberyReplayPage : public GamePage, public ActionListener {
        
    public:
        
        FetchRobberyReplayPage();
        
        virtual ~FetchRobberyReplayPage();

        // must be called before makeActive
        void setLogID( int inID );
        

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
        
        int mLogID;

        void clearRecord();
    };



#endif
