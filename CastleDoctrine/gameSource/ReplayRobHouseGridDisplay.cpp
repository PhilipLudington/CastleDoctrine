#include "ReplayRobHouseGridDisplay.h"


#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"

#include "minorGems/util/stringUtils.h"


extern Font *mainFont;

extern double frameRateFactor;


#define STEP_DELAY (int)( 32 * frameRateFactor )

#define BUTTON_X -8



ReplayRobHouseGridDisplay::ReplayRobHouseGridDisplay( double inX, double inY )
        : RobHouseGridDisplay( inX, inY ),
          mStepButton( mainFont, BUTTON_X, -5, 
                       translate( "step" ) ),
          mFasterButton( mainFont, BUTTON_X, -5, 
                         translate( "faster" ) ),
          mPlayButton( mainFont, BUTTON_X, -3.5, 
                       translate( "play" ) ),
          mStopButton( mainFont, BUTTON_X, -2, 
                       translate( "stop" ) ),
          mRestartButton( mainFont, BUTTON_X, -0.5, 
                          translate( "restart" ) ),
          mVisibilityButton( mainFont, BUTTON_X, 5, 
                             translate( "toggleVisibility" ) ),
          mPlaying( false ),
          mSpeedFactor( 1 ),
          mFullSpeed( false ),
          mStepsUntilNextPlayStep( 0 ),
          mVisibilityToggle( false ),
          mJustRestarted( false ),
          mToolIDJustPicked( -1 ),
          mPanning( false ),
          mOriginalHouseMap( NULL ),
          mOriginalWifeMoney( 0 ),
          mOriginalMoveList( NULL ) {

    mStopButton.setVisible( false );
    mFasterButton.setVisible( false );
    
    addComponent( &mStepButton );
    addComponent( &mPlayButton );
    addComponent( &mStopButton );
    addComponent( &mFasterButton );
    addComponent( &mRestartButton );
    addComponent( &mVisibilityButton );
    
    mStepButton.addActionListener( this );
    mPlayButton.addActionListener( this );
    mStopButton.addActionListener( this );
    mFasterButton.addActionListener( this );
    mRestartButton.addActionListener( this );
    mVisibilityButton.addActionListener( this );
    

    
    mVisibilityButton.setMouseOverTip( translate( "toggleVisibilityTip" ) );

    // for panning around with arrow keys when paused
    mStepsBetweenHeldKeyRepeat = 4;
    mAllowKeyRepeatAcceleration = true;
    }


        
ReplayRobHouseGridDisplay::~ReplayRobHouseGridDisplay() {
    if( mOriginalHouseMap != NULL ) {
        delete [] mOriginalHouseMap;
        }

    if( mOriginalMoveList != NULL ) {
        delete [] mOriginalMoveList;
        }

    clearReplayMoveList();
    }


void ReplayRobHouseGridDisplay::clearReplayMoveList() {
    for( int i=0; i<mReplayMoveList.size(); i++ ) {
        delete [] *( mReplayMoveList.getElement( i ) );
        }
    mReplayMoveList.deleteAll();
    }



void ReplayRobHouseGridDisplay::setHouseMap( const char *inHouseMap ) {
    if( mOriginalHouseMap != NULL ) {
        delete [] mOriginalHouseMap;
        }
    mOriginalHouseMap = stringDuplicate( inHouseMap );
    
    RobHouseGridDisplay::setHouseMap( inHouseMap );
    }



void ReplayRobHouseGridDisplay::setWifeMoney( int inMoney ) {
    mOriginalWifeMoney = inMoney;

    RobHouseGridDisplay::setWifeMoney( inMoney );
    }



void ReplayRobHouseGridDisplay::setMoveList( char *inMoveList ) {
    clearReplayMoveList();

    char *oldList = mOriginalMoveList;
    mOriginalMoveList = stringDuplicate( inMoveList );

    if( oldList != NULL ) {
        delete [] oldList;
        }


    if( strcmp( inMoveList, "#" ) != 0 ) {
        // a non-empty move list, split it
        int numMoves;
        char **moves = split( inMoveList, "#", &numMoves );

        mReplayMoveList.appendArray( moves, numMoves );
    
        delete [] moves;
        }
    

    // prepare for new playback
    mPlaying =  false;
    mFullSpeed = false;
    mSkipShadowComputation = false;
    
    mStepButton.setVisible( true );
    mPlayButton.setVisible( true );
    mStopButton.setVisible( false );
    mFasterButton.setVisible( false );
    mRestartButton.setVisible( false );

    hideRobber( false );

    mPanning = false;
    }




char ReplayRobHouseGridDisplay::getJustRestarted() {
    char temp = mJustRestarted;
    mJustRestarted = false;
    
    return temp;
    }



int ReplayRobHouseGridDisplay::getToolIDJustPicked() {
    int temp = mToolIDJustPicked;
    mToolIDJustPicked = -1;

    return temp;
    }




void ReplayRobHouseGridDisplay::playAtFullSpeed() {
    mPlaying = true;
    mFullSpeed = true;
    mSkipShadowComputation = true;

    mSpeedFactor = STEP_DELAY;
    mStepsUntilNextPlayStep = STEP_DELAY / mSpeedFactor; 

    mStopButton.setVisible( true );
    mFasterButton.setVisible( false );
    mPlayButton.setVisible( false );
    mStepButton.setVisible( false );
    }



char ReplayRobHouseGridDisplay::getMoveListExhausted() {
    if( mReplayMoveList.size() == 0 ) {
        return true;
        }
    return false;
    }



void ReplayRobHouseGridDisplay::step() {
    RobHouseGridDisplay::step();
    
    if( mPlaying ) {
        mStepsUntilNextPlayStep --;
        
        if( mStepsUntilNextPlayStep == 0 ) {
            takeStep();
            
            mStepsUntilNextPlayStep = STEP_DELAY / mSpeedFactor;
            }
        }
    
    }



void ReplayRobHouseGridDisplay::takeStep() {
    mRestartButton.setVisible( true );
    endPanning();
    if( mReplayMoveList.size() > 0 ) {
        
        char *move = *( mReplayMoveList.getElement( 0 ) );
        
        char shouldDeleteMove = true;

        if( strlen( move ) > 0 ) {
            if( move[0] == 'm' ) {
                // player movement to new index
                int newIndex;
                sscanf( move, "m%d", &newIndex );
                
                moveRobber( newIndex );
                shouldDeleteMove = true;
                }
            else if( move[0] == 't' ) {
                int toolID, targetIndex;
                
                sscanf( move, "t%d@%d", &toolID, &targetIndex );
                
                if( mCurrentTool == -1 ) {
                    
                    // display that tool is being used
                    startUsingTool( toolID );
                    mToolIDJustPicked = toolID;
                    fireActionPerformed( this );
                    
                    // save tool move for next step (further processing)
                    shouldDeleteMove = false;
                    }
                else if( mToolTargetPickedFullIndex == -1 ) {
                    // display where user is clicking
                    setPickedTargetHighlight( targetIndex );
                    
                    // save tool move for next step (further processing)
                    shouldDeleteMove = false;
                    }
                else {
                    // already started using tool last step
                    // AND shown where user clicked

                    // actually use it this step

                    applyCurrentTool( targetIndex );
                    fireActionPerformed( this );

                    shouldDeleteMove = true;
                    }
                }
            else if( move[0] == 'L' ) {
                robberTriedToLeave();
                hideRobber( true );
                shouldDeleteMove = true;
                }
            else if( move[0] == 'S' ) {
                mRobberState = 99;
                mDead = true;
                shouldDeleteMove = true;
                }
            }

        if( shouldDeleteMove ) {
            mReplayMoveList.deleteElement( 0 );
            
            delete [] move;
            }
        
        }

    if( mReplayMoveList.size() == 0 ) {
        // auto stop, no more steps allowed
        mPlaying =  false;
        
        mStepButton.setVisible( false );
        mPlayButton.setVisible( false );
        mStopButton.setVisible( false );
        mFasterButton.setVisible( false );
        }
    }



void ReplayRobHouseGridDisplay::pointerDrag( float inX, float inY ) {
    pointerOver( inX, inY );
    
    // ignore mouse (beyond tool tips)
    }



void ReplayRobHouseGridDisplay::pointerUp( float inX, float inY ) {
    pointerOver( inX, inY );
        
    // ignore mouse (beyond tool tips)
    }




void ReplayRobHouseGridDisplay::specialKeyDown( int inKeyCode ) {
    // arrow keys for panning around while paused

    if( mPlaying || ! mVisibilityToggle ) {
        // ignore panning while playing or while shroud on
        return;
        }
    

    // hijack key hold-down behavior from HouseGridDisplay
    if( mSpecialKeysHeldSteps[ inKeyCode ] == 0 ) {
        // not already down

        mSpecialKeysHeldSteps[ inKeyCode ] = 1;
        }

    // clear hold status of all other special keys
    for( int i=0; i<MG_KEY_LAST_CODE+1; i++ ) {
        if( i != inKeyCode ) {
            mSpecialKeysHeldSteps[i] = 0;
            }
        }
    

    int newOffsetX = mSubMapOffsetX;
    int newOffsetY = mSubMapOffsetY;


    if( inKeyCode == MG_KEY_LEFT ) {
        if( newOffsetX > 0 ) {
            newOffsetX --;
            }
        }
    else if( inKeyCode == MG_KEY_RIGHT ) {
        if( newOffsetX + HOUSE_D < mFullMapD ) {
            newOffsetX ++;
            }
        }
    else if( inKeyCode == MG_KEY_DOWN ) {
        if( newOffsetY > 0 ) {
            newOffsetY --;
            }
        }
    else if( inKeyCode == MG_KEY_UP ) {
        if( newOffsetY + HOUSE_D < mFullMapD ) {
            newOffsetY ++;
            }
        }

    if( newOffsetX != mSubMapOffsetX ||
        newOffsetY != mSubMapOffsetY ) {
        
        if( !mPanning ) {    
            mPanning = true;
            mSavedVisibleOffsetX = mSubMapOffsetX;
            mSavedVisibleOffsetY = mSubMapOffsetY;
            }
        

        setVisibleOffset( newOffsetX, newOffsetY );
        }
    }

        
        
void ReplayRobHouseGridDisplay::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == &mPlayButton ) {
        takeStep();
        mPlaying = true;
        mSpeedFactor = 1;
        mStepsUntilNextPlayStep = STEP_DELAY / mSpeedFactor;     

        mStopButton.setVisible( true );
        mFasterButton.setVisible( true );
        mPlayButton.setVisible( false );
        mStepButton.setVisible( false );
       }
    else if( inTarget == &mFasterButton ) {

        if( STEP_DELAY / mSpeedFactor >= 2 ) {
            mSpeedFactor *= 2;
            }
        if( STEP_DELAY / mSpeedFactor < 2 ) {
            mFasterButton.setVisible( false );
            }
        }
    else if( inTarget == &mStopButton ) {
        mPlaying = false;
        mStopButton.setVisible( false );
        mFasterButton.setVisible( false );
        mPlayButton.setVisible( true );
        mStepButton.setVisible( true );
        }
    else if( inTarget == &mStepButton ) {
        takeStep();
        }
    else if( inTarget == &mRestartButton ) {
        char *newHouseMap = stringDuplicate( mOriginalHouseMap );
        setHouseMap( newHouseMap );
        delete [] newHouseMap;
        
        char *newMoveList = stringDuplicate( mOriginalMoveList );
        setMoveList( newMoveList );
        delete [] newMoveList;
        
        RobHouseGridDisplay::setWifeMoney( mOriginalWifeMoney );
        
        stopUsingTool();

        mRestartButton.setVisible( false );
        mStopButton.setVisible( false );
        mFasterButton.setVisible( false );

        mPlayButton.setVisible( true );
        mStepButton.setVisible( true );
        mPlaying = false;
        mJustRestarted = true;
        fireActionPerformed( this );
        }
    else if( inTarget == &mVisibilityButton ) {
        endPanning();
        mVisibilityToggle = ! mVisibilityToggle;
        mForceAllTileToolTips = mVisibilityToggle;
        mBlockVisibilityFading = mVisibilityToggle;
        recomputeVisibility();
        }
    }



void ReplayRobHouseGridDisplay::recomputeVisibility() {

    RobHouseGridDisplay::recomputeVisibility();
    
    if( mVisibilityToggle ) {    
        // all visible during playback
        // though keep mTileVisbleMap set to true visibility for proper
        // tool target selection.

        int i = 0;
        for( int y=0; y<HOUSE_D * VIS_BLOWUP; y++ ) {
            for( int x=0; x<HOUSE_D * VIS_BLOWUP; x++ ) {

                mTargetVisibleMap[i] = true;
                mTargetVisibleUnderSlipMap[i] = true;
                i++;
                }
            }
        }
    
    }




void ReplayRobHouseGridDisplay::endPanning() {
    if( mPanning ) {

        char actuallyJumping = false;
        if( mSubMapOffsetX != mSavedVisibleOffsetX ||
            mSubMapOffsetY != mSavedVisibleOffsetY ) {
            actuallyJumping = true;
            }
        
        // jump back
        setVisibleOffset( mSavedVisibleOffsetX, mSavedVisibleOffsetY );
        
        mPanning = false;
        
        if( actuallyJumping ) {
            
            // black out actual visibility to avoid weird
            // overlapping effects as our view window jumps back
            // let visibility fade to desired visibility from all black
            for( int i=0; 
                 i<HOUSE_D * HOUSE_D * VIS_BLOWUP * VIS_BLOWUP; 
                 i++ ) {
            
                mVisibleMap[i] = 255;
                mVisibleUnderSlipMap[i] = 255;
                }

            // DON'T black out if we're not actually jumping anywhere
            // to preserve the nice fade-in of shroud in that case.
            }
        }
    }

