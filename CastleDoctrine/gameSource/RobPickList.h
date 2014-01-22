#ifndef ROB_PICK_LIST_INCLUDED
#define ROB_PICK_LIST_INCLUDED



#include "GamePage.h"
#include "PageComponent.h"

#include "SpriteButton.h"

#include "TextField.h"
#include "TextButton.h"



typedef struct HouseRecord {
        // might be homeID (in case of houses-to-rob list)
        //       or logID (in the case of a list of past robberies to replay)
        int uniqueID;
        // full name with "_" for spaces as sent by server
        char *rawCharacterName;
        // trimmed and spaced name for human display
        char *characterName;
        char *lastRobberName;
        
        int bountyFlag;
        int lootValue;
        

        // records for rob pick and tape list have different stats
        // but both are ints, so can be scanned and presented
        // this may be overkill.... but I've never gotten to use a union before
        union {
                int robAttempts;
                int numSteps;
                int eitherStat;
            } stat;
        

        int robberDeaths;
        int flag;
        
        char selected;
        doublePair position;

        char draw;
    } HouseRecord;


// fires actionPerformed when selection changes
class RobPickList : public PageComponent, public ActionListener, 
                    public ActionListenerList {
        
    public:
        
        
        RobPickList( double inX, double inY,
                     // if false, show houses-to-rob list
                     // if true, show list of successful robberies to view
                     char inRobberyLog,
                     GamePage *inParentPage );
        

        virtual ~RobPickList();
        
        
        virtual void actionPerformed( GUIComponent *inTarget );

        
        // fetch new results from server
        virtual void refreshList( char inPreserveSearch = false,
                                  char inPreservePosition = false );
        
        
        // NULL if nothing selected
        // destroyed internally
        virtual HouseRecord *getSelectedHouse();
        

        virtual void step();
        
        virtual void draw();


        virtual void pointerUp( float inX, float inY );
        virtual void pointerMove( float inX, float inY );
        virtual void pointerDrag( float inX, float inY );
        virtual void pointerDown( float inX, float inY );
        


    protected:
        
        // which item hit by mouse?
        // NULL on miss
        HouseRecord *getHitRecord( float inX, float inY );
        void setTip( HouseRecord *inRecord );


        GamePage *mParentPage;

        int mArePagesLeft;
        
        int mCurrentSkip;
        
        int mWebRequest;
        
        int mProgressiveDrawSteps;
        
        char mRobberyLog;

        SimpleVector<HouseRecord> mHouseList;

        SpriteHandle mSkullSprite;

        void clearHouseList();

        char mHover;
        
        
        SpriteButton mUpButton;
        SpriteButton mDownButton;

        TextField mSearchField;
        TextButton mFilterButton;
        TextButton mIgnoreButton;
        TextButton mClearIgnoreListButton;
        
        char *mAppliedSearchWords;

        char mIgnoreSet;
        char *mIgnoreTarget;

        char mAnyPossiblyIgnored;
        
        char mClearIgnoreListSet;
        
        
    };



#endif

        
