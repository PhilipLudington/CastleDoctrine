#ifndef REPLAY_ROB_HOUSE_GRID_DISPLAY_INCLUDED
#define REPLAY_ROB_HOUSE_GRID_DISPLAY_INCLUDED


#include "RobHouseGridDisplay.h"

#include "TextButton.h"



// grid that supports replaying a move list
// fires actionPerformed when replay is restarted to beginning, 
// or tool picked/used
class ReplayRobHouseGridDisplay : public RobHouseGridDisplay, 
                                  public ActionListener {
    
    public:

        ReplayRobHouseGridDisplay( double inX, double inY );
        
        virtual ~ReplayRobHouseGridDisplay();

        // override
        virtual void setWifeMoney( int inMoney );

        // override save a copy of map for rewind
        virtual void setHouseMap( const char *inHouseMap );

        // must be called AFTER setting house map

        // list as a whitespace-free string
        // destroyed by caller
        void setMoveList( char *inMoveList );
        
        // returns true once if just restarted
        // called to check why fireActionPerformed happed
        char getJustRestarted();
        

        // can check whether a tool has been picked since the last
        // call to getToolIDJustPicked
        // returns ID of pickedtool, or -1
        int getToolIDJustPicked();
        

        // for automated robbery simulation
        // switch to fastest playback speed (one move per step() ) and
        // initiate play
        // AND disable visibility computations
        virtual void playAtFullSpeed();
        
        virtual char getMoveListExhausted();
        

        virtual void step();


        // override to ignore arrow key movement
        virtual void specialKeyDown( int inKeyCode );
        
        
        virtual void actionPerformed( GUIComponent *inTarget );
        

        // override from RobHouseGridDisplay so that tool targeting not
        // triggered by mouse
        virtual void pointerDrag( float inX, float inY );

        virtual void pointerUp( float inX, float inY );


    protected:
        
        
        // override to make EVERYTHING visible if visibility toggled
        virtual void recomputeVisibility();

        
        TextButton mStepButton;
        TextButton mFasterButton;
        TextButton mPlayButton;
        TextButton mStopButton;
        TextButton mRestartButton;
        
        TextButton mVisibilityButton;

        char mPlaying;
        int mSpeedFactor;
        
        char mFullSpeed;
        
        
        int mStepsUntilNextPlayStep;

        char mVisibilityToggle;
        
        char mJustRestarted;
        
        int mToolIDJustPicked;

        void takeStep();
        
        // to jump back from panning around when next step taken
        char mPanning;
        int mSavedVisibleOffsetX;
        int mSavedVisibleOffsetY;

        void endPanning();
        

        SimpleVector<char *> mReplayMoveList;

        void clearReplayMoveList();

        // for restart
        char *mOriginalHouseMap;
        int mOriginalWifeMoney;
        char *mOriginalMoveList;
    };



#endif
