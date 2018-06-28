#include "RobHouseGridDisplay.h"

#include "minorGems/game/gameGraphics.h"
#include "minorGems/game/game.h"
#include "minorGems/game/drawUtils.h"

#include "minorGems/util/stringUtils.h"



#include "minorGems/game/Font.h"

extern Font *mainFont;



#include "FastBoxBlurFilter.h"
#include "houseObjects.h"
#include "houseTransitions.h"
#include "tools.h"

#include <math.h>
#include <limits.h>



extern double frameRateFactor;



RobHouseGridDisplay::RobHouseGridDisplay( double inX, double inY )
        : HouseGridDisplay( inX, inY ),
          mSuccess( 0 ),
          mDead( false ),
          mDeathSourceID( -1 ),
          mDeathSourceState( 1 ),
          mFamilyAndMobilesProcessedAtEnd( false ),
          mLeaveDisplayCanBeShown( true ),
          mLeaveSprite( loadSprite( "left.tga" ) ),
          mCurrentTool( -1 ),
          mToolJustUsed( false ),
          mSafeMoveMode( false ),
          mSafeMoveProposed( false ),
          mSafeMoveIndex( -1 ),
          mSafeMoveConfirmed( false ),
          mSafeUpSprite( loadSprite( "moveUp.tga" ) ),
          mSafeLeftSprite( loadSprite( "moveLeft.tga" ) ),
          mSafeDownSprite( loadSprite( "moveDown.tga" ) ),
          mSafeRightSprite( loadSprite( "moveRight.tga" ) ),
          mBlockVisibilityFading( false ),
          mHouseMapCellFadesWorking( NULL ),
          mForceAllTileToolTips( false ),
          mHouseMapMobileStartingPositions( NULL ),
          mHouseMapFinalIDs( NULL ),
          mHouseMapFinalCellStates( NULL ),
          mHouseMapMobileFinalIDs( NULL ),
          mHouseMapMobileFinalCellStates( NULL ),
          mRobberStoleFromWife( false ) {

    for( int i=0; i<VIS_BLOWUP * HOUSE_D * VIS_BLOWUP * HOUSE_D; i++ ) {
        mVisibleMap[i] = 0;
        mVisibleUnderSlipMap[i] = 0;
        }
    for( int i=0; i<HOUSE_D * HOUSE_D; i++ ) {
        mShroudPunchThroughMap[i] = 255;
        }
    
    mBottomRowNonConnectedFaded = true;
    }



RobHouseGridDisplay::~RobHouseGridDisplay() {
    clearMoveList();

    freeSprite( mLeaveSprite );

    freeSprite( mSafeUpSprite );
    freeSprite( mSafeLeftSprite );
    freeSprite( mSafeDownSprite );
    freeSprite( mSafeRightSprite );

    if( mHouseMapMobileStartingPositions != NULL ) {
        delete [] mHouseMapMobileStartingPositions;
        }

    if( mHouseMapFinalIDs != NULL ) {
        delete [] mHouseMapFinalIDs;
        }
    if( mHouseMapFinalCellStates != NULL ) {
        delete [] mHouseMapFinalCellStates;
        }

    if( mHouseMapMobileFinalIDs != NULL ) {
        delete [] mHouseMapMobileFinalIDs;
        }
    if( mHouseMapMobileFinalCellStates != NULL ) {
        delete [] mHouseMapMobileFinalCellStates;
        }

    if( mHouseMapCellFadesWorking != NULL ) {
        delete [] mHouseMapCellFadesWorking;
        }
    }



void RobHouseGridDisplay::clearMoveList() {
    for( int i=0; i<mMoveList.size(); i++ ) {
        delete [] *( mMoveList.getElement( i ) );
        }
    mMoveList.deleteAll();
    }



int RobHouseGridDisplay::getSuccess() {
    return mSuccess;
    }



char RobHouseGridDisplay::getDead() {
    return mDead;
    }



int RobHouseGridDisplay::getDeathSourceID() {
    return mDeathSourceID;
    }


int RobHouseGridDisplay::getDeathSourceState() {
    return mDeathSourceState;
    }



char RobHouseGridDisplay::getWifeKilledRobber() {
    if( mDead ) {
        return isPropertySet( mDeathSourceID, mDeathSourceState, wife );
        }
    else {
        return false;
        }
    }



char RobHouseGridDisplay::getWifeRobbed() {
    return mRobberStoleFromWife;
    }



char RobHouseGridDisplay::getWifeKilled() {
    int numFamily = mFamilyStatus.size();
    
    for( int i=0; i<numFamily; i++ ) {
        if( *( mFamilyStatus.getElement( i ) ) == 0 ) {
            // killed
            
            // is wife?
            int objectID = *( mFamilyObjects.getElement( i ) );
            
            if( isPropertySet( objectID, 0, wife ) ) {
                return true;
                }
            }
        }
    

    return false;
    }



int RobHouseGridDisplay::getFamilyKilledCount() {
    int numFamily = mFamilyStatus.size();
    
    int numKilled = 0;
    
    for( int i=0; i<numFamily; i++ ) {
        if( *( mFamilyStatus.getElement( i ) ) == 0 ) {
            // killed
            numKilled ++;
            }
        }
    return numKilled;
    }




char *RobHouseGridDisplay::getMoveList() {
    if( mSuccess == 0 && ! mDead ) {
        // must have committed suicide
        mMoveList.push_back( stringDuplicate( "S" ) );
        }

    if( mMoveList.size() == 0 ) {
        return stringDuplicate( "#" );
        }
    


    char **moveArray = mMoveList.getElementArray();
    
    char *moveString = join( moveArray, mMoveList.size(), "#" );
    
    delete [] moveArray;

    return moveString;
    }



void RobHouseGridDisplay::startUsingTool( int inToolID ) {
    mSafeMoveConfirmed = false;
    mSafeMoveProposed = false;
    mSafeMoveIndex = -1;
    clearSpecialHighlights();
    
    if( ! getToolInRange( inToolID ) ) {
        mForbiddenMoveHappened = true;
        return;
        }

    mCurrentTool = inToolID;

    if( mCurrentTool != -1 ) {
        int reach = getToolReach( mCurrentTool );
        
        SimpleVector<int> hitSquares;
        hitSquares.push_back( mRobberIndex );
        
        int dX[] = { -1,  0,  1,  0 };
        int dY[] = {  0,  1,  0, -1 };
        

        for( int i=0; i<reach; i++ ) {
            // expand by one
            int existingSize = hitSquares.size();
            for( int j=0; j<existingSize; j++ ) {
                
                int index = *( hitSquares.getElement( j ) );
                
                if( isMapPropertySet( index, blocking ) ) {
                    // blocking objects can be reached by a tool, but
                    // they can't pass reachability through to their neighbors
                    continue;
                    }

                int y = index / mFullMapD;
                int x = index % mFullMapD;
                
                for( int d=0; d<4; d++ ) {
                    int yNew = y + dY[d];
                    int xNew = x + dX[d];
                    
                    if( xNew >= 0 && xNew < mFullMapD &&
                        yNew >= 0 && yNew < mFullMapD ) {
                        
                        int indexNew = yNew * mFullMapD + xNew;

                        if( hitSquares.getElementIndex( indexNew ) == -1 ) {
                            // not already hit, add it?
                            
                            // but only if it's visible
                            int subIndexNew = fullToSub( indexNew );
                            
                            if( subIndexNew != -1 ) {
                                
                                if( mTileVisibleMap[ subIndexNew ] ) {
                                    hitSquares.push_back( indexNew );
                                    }
                                }
                            }
                        }
                    }
                }
            }

        // don't draw highlight over robber
        hitSquares.deleteElementEqualTo( mRobberIndex );
        
        // don't draw highlight over welcome mat
        hitSquares.deleteElementEqualTo( mStartIndex );
        
        // filter target squares based on which targets can actually
        // be changed by the current tool
        
        for( int i=0; i<hitSquares.size(); i++ ) {
            int index = *( hitSquares.getElement( i ) );
            
            if( checkTransition( mHouseMapIDs[ index ],
                                 mHouseMapCellStates[ index ],
                                 inToolID,
                                 -1 )
                != 
                mHouseMapCellStates[ index ]
                ||
                checkTransition( mHouseMapMobileIDs[ index ],
                                 mHouseMapMobileCellStates[ index ],
                                 inToolID,
                                 -1 )
                != 
                mHouseMapMobileCellStates[ index ] ) {
                // keep target
                }
            else {
                hitSquares.deleteElement( i );
                i--;
                }
            }
        

        setTargetHighlights( &hitSquares );
        }
    }



void RobHouseGridDisplay::stopUsingTool() {
    mCurrentTool = -1;

    SimpleVector<int> emptyVector;
    setTargetHighlights( &emptyVector );
    }



char RobHouseGridDisplay::getToolJustUsed() {
    char temp = mToolJustUsed;
    mToolJustUsed = false;

    return temp;
    }




void RobHouseGridDisplay::setSafeMoveMode( char inOn ) {
    mSafeMoveMode = inOn;
    mSafeMoveProposed = false;
    mSafeMoveIndex = -1;
    mSafeMoveConfirmed = false;
    clearSpecialHighlights();
    }



void RobHouseGridDisplay::setHouseMap( const char *inHouseMap ) {
    mSuccess = 0;
    mDead = false;
    mDeathSourceID = -1;
    mDeathSourceState = 1;

    mFamilyAndMobilesProcessedAtEnd = false;

    stopUsingTool();
    mToolJustUsed = false;
    
    clearMoveList();

    HouseGridDisplay::setHouseMap( inHouseMap );    

    if( mHouseMapMobileStartingPositions != NULL ) {
        delete [] mHouseMapMobileStartingPositions;
        }

    mHouseMapMobileStartingPositions = new int[ mNumMapSpots ];
    
    for( int i=0; i<mNumMapSpots; i++ ) {
    
        if( mHouseMapMobileIDs[i] != 0 ) {
            mHouseMapMobileStartingPositions[i] = i;
            }
        else {
            mHouseMapMobileStartingPositions[i] = -1;
            }
        }
    

    if( mHouseMapCellFadesWorking != NULL ) {
        delete [] mHouseMapCellFadesWorking;
        }
    mHouseMapCellFadesWorking = new float[ mNumMapSpots ];
    


    mFamilyExitPathProgress.deleteAll();
    mFamilyObjects.deleteAll();
    mFamilyStatus.deleteAll();
    mFamilySeenRobber.deleteAll();

    if( ! mAllFamilyObjectsHaveExitPath ) {
        // some family killed since owner last edited this house.

        // try again, this time letting them walk through
        // dead bodies, which will mean that during robbery, they
        // will walk up to dead bodies and stop (instead of being
        // frozen in place because of a far-away blocking body)
        
        printf( "All family don't have paths.  Looking again...\n" );
        mDeadFamilyBlocking = false;
        checkExitPaths();
        mDeadFamilyBlocking = true;
        }
    
    
    int numPaths = mFamilyExitPaths.size();
    
    for( int i=0; i<numPaths; i++ ) {
        mFamilyExitPathProgress.push_back( 0 );
        GridPos *path = *( mFamilyExitPaths.getElement( i ) );
        
        GridPos start = path[0];
        
        int startIndex = start.y * mFullMapD + start.x;
        
        mFamilyObjects.push_back( mHouseMapIDs[ startIndex ] );
        mFamilyStatus.push_back( 1 );
        mFamilySeenRobber.push_back( false );
        }
    

    mRobberStoleFromWife = false;

    // switch all unstuck
    // to "1" state for presentation to robber
    resetToggledStates( 1 );

    // robber outfit
    mRobberState = 1;
    
    for( int i=0; i<HOUSE_D * HOUSE_D * VIS_BLOWUP * VIS_BLOWUP; i++ ) {
        mVisibleMap[i] = 255;
        mVisibleUnderSlipMap[i] = 255;
        mTargetVisibleMap[i] = false;
        mTargetVisibleUnderSlipMap[i] = false;
        }
    for( int i=0; i<HOUSE_D * HOUSE_D; i++ ) {
        mShroudPunchThroughMap[i] = 255;
        mTargetShroudPunchThroughMap[i] = false;
        }
    
    for( int i=0; i<mNumMapSpots; i++ ) {
        mHouseMapCellFades[i] = 0;
        mHouseMapMobileCellFades[i] = 0;
        mHouseMapCellFadesWorking[i] = 0;
        }

    // initial transitions (like for power that starts out on, etc)
    freezeMobileObjects( true );
    applyTransitionsAndProcess();
    freezeMobileObjects( false );
    }




char *RobHouseGridDisplay::getHouseMap() {
    
    if( mHouseMapFinalIDs != NULL &&
        mHouseMapFinalCellStates != NULL ) {
        
        memcpy( mHouseMapIDs, mHouseMapFinalIDs, 
                mNumMapSpots * sizeof( int ) );

        memcpy( mHouseMapCellStates, mHouseMapFinalCellStates,
                mNumMapSpots * sizeof( int ) );
        
    
        delete [] mHouseMapFinalIDs;
        delete [] mHouseMapFinalCellStates;

        mHouseMapFinalIDs = NULL;
        mHouseMapFinalCellStates = NULL;
        }
    if( mHouseMapMobileFinalIDs != NULL &&
        mHouseMapMobileFinalCellStates != NULL ) {
        
        memcpy( mHouseMapMobileIDs, mHouseMapMobileFinalIDs, 
                mNumMapSpots * sizeof( int ) );

        memcpy( mHouseMapMobileCellStates, mHouseMapMobileFinalCellStates,
                mNumMapSpots * sizeof( int ) );
        
    
        delete [] mHouseMapMobileFinalIDs;
        delete [] mHouseMapMobileFinalCellStates;

        mHouseMapMobileFinalIDs = NULL;
        mHouseMapMobileFinalCellStates = NULL;
        }
    
    return HouseGridDisplay::getHouseMap();
    }




void RobHouseGridDisplay::startWifeSearchForDeadChild( int inIndex ) {
    int numPaths = mFamilyExitPaths.size();
    for( int i=0; i<numPaths; i++ ) {
        int progress = *( mFamilyExitPathProgress.getElement( i ) );
        
        GridPos *path = *( mFamilyExitPaths.getElement( i ) );
        
        int status = *( mFamilyStatus.getElement( i ) );
        
        // still in house, alive
        if( status == 1 ) {
            
            int oldIndex = posToIndex( path[progress] );

            // wife and still alive
            if( isPropertySet( mHouseMapIDs[ oldIndex ],
                               mHouseMapCellStates[ oldIndex ], wife ) ) {

                char *blockedMap = getBlockedMap();
                
                // make dead child location unblocked
                // wife will walk to an adjacent square and stop
                blockedMap[ inIndex ] = false;
                
                GridPos startPos = 
                    { oldIndex % mFullMapD, oldIndex / mFullMapD };
                
                GridPos goalPos = 
                    { inIndex % mFullMapD, inIndex / mFullMapD };

                int numStepsToGoal;
                GridPos *fullPath;
                
                char found = pathFind( mFullMapD, mFullMapD,
                                       blockedMap, 
                                       startPos, goalPos, 
                                       &numStepsToGoal,
                                       &fullPath );
            
                if( found && numStepsToGoal != 0 ) {
                    
                    // splice with beginning of old path

                    int newPathLength = progress + numStepsToGoal;
                    
                    GridPos *newPath = 
                        new GridPos[ newPathLength ];
                    
                    memcpy( newPath, path, 
                            ( progress + 1 ) * sizeof( GridPos ) );
                    
                    memcpy( &( newPath[ progress + 1 ] ),
                            &( fullPath[ 1 ] ),
                            ( numStepsToGoal - 1 ) * sizeof( GridPos ) );
                    


                    delete [] *( mFamilyExitPaths.getElement( i ) );
                    delete [] fullPath;
                    
                    *( mFamilyExitPaths.getElement( i ) ) = newPath;
                    
                    *( mFamilyExitPathLengths.getElement( i ) ) = 
                        newPathLength;

                    // new path, same progress along it (wife at
                    // splice point)
                    *( mFamilyExitPathProgress.getElement( i ) ) = progress;

                    // force wife to "see robber" so that she starts moving
                    *( mFamilySeenRobber.getElement( i ) ) = true;
                    }
                else {
                    // no path found
                    // keep going to exit (original path)
                    }
                
                delete [] blockedMap;
                
                break;
                }
            }
        }       
    }



void RobHouseGridDisplay::applyTransitionsAndProcess() {

    // check for family deaths, regardless of whether mobiles frozen
    int numPaths = mFamilyExitPaths.size();
    for( int i=0; i<numPaths; i++ ) {
        int progress = *( mFamilyExitPathProgress.getElement( i ) );
        
        int pathLength = *( mFamilyExitPathLengths.getElement( i ) );
        
        GridPos *path = *( mFamilyExitPaths.getElement( i ) );
        
        if( pathLength == 1 || progress < pathLength - 1 ) {
            
            int oldIndex = posToIndex( path[progress] );

            if( isPropertySet( mHouseMapIDs[ oldIndex ],
                               mHouseMapCellStates[ oldIndex ],
                               deadFamily ) ) {
                // killed!
                // don't move
                    
                if( *( mFamilyStatus.getElement( i ) ) != 0 ) {
                    // just died
                    
                    if( isPropertySet( mHouseMapIDs[oldIndex],
                                       mHouseMapCellStates[oldIndex],
                                       son ) ||
                        isPropertySet( mHouseMapIDs[oldIndex],
                                       mHouseMapCellStates[oldIndex],
                                       daughter ) ) {

                        startWifeSearchForDeadChild( oldIndex );
                        }

                    // dead status
                    *( mFamilyStatus.getElement( i ) ) = 0;
                    }
                }
            }        
        }
    



    if( ! areMobilesFrozen() ) {
        
        // first, move each family member
        int numPaths = mFamilyExitPaths.size();
        for( int i=0; i<numPaths; i++ ) {
            int progress = *( mFamilyExitPathProgress.getElement( i ) );
        
            int pathLength = *( mFamilyExitPathLengths.getElement( i ) );
        
            GridPos *path = *( mFamilyExitPaths.getElement( i ) );

            int objectID = *( mFamilyObjects.getElement( i ) );

            char seenRobber = *( mFamilySeenRobber.getElement( i ) );
            
            if( !seenRobber ) {
                // family member hasn't seen robber, not moving yet
                // skip them
                continue;
                }
            
            if( pathLength == 1 || progress < pathLength - 1 ) {
            
                int oldIndex = posToIndex( path[progress] );

                if( isPropertySet( mHouseMapIDs[oldIndex],
                                   mHouseMapCellStates[ oldIndex ], 
                                   deadFamily ) ) {
                    // killed!
                    // don't move
                    }
                else if( progress < pathLength - 1 ) {
                    

                    progress ++;

                    int newIndex = posToIndex( path[progress] );
                    
                    // make sure it's clear
                    if( newIndex != mRobberIndex &&
                        mHouseMapIDs[newIndex] == 0 ) {
                        // move along
                
                        mHouseMapIDs[newIndex] = 
                            mHouseMapIDs[oldIndex];
                    
                        mHouseMapCellStates[newIndex] = 
                            mHouseMapCellStates[oldIndex];
                
                        // leave empty where we used to be
                        mHouseMapIDs[oldIndex] = 0;
                        mHouseMapCellStates[oldIndex] = 1;

                        if( mHouseMapCellFades[newIndex] != 
                            mHouseMapCellFades[oldIndex] ) {
                            // copy old visibility fade so that family member
                            // doesn't blink out of existence or blink into
                            // existence
                            // (instead, they will fade out/in at new location)
                            mHouseMapCellFades[newIndex] = 
                                mHouseMapCellFades[oldIndex];
                            }
                        
                        *( mFamilyExitPathProgress.getElement( i ) ) = 
                            progress;
                        }    
                    }
                }
            else {    
                // else already done...
            
                // if standing on entrance, remove from map
                // (escape during this step)
                int oldIndex = posToIndex( path[progress] );

                if( oldIndex == mStartIndex &&
                    mHouseMapIDs[oldIndex] == objectID ) {
                    
                    mHouseMapIDs[oldIndex] = 0;
                    mHouseMapCellStates[oldIndex] = 1;
                    
                    // escaped status
                    *( mFamilyStatus.getElement( i ) ) = 2;
                    }
                }
            
            
            }
        }
    
 


    applyTransitions( mHouseMapIDs, mHouseMapCellStates, 
                      mHouseMapMobileIDs, mHouseMapMobileCellStates,
                      mHouseMapMobileStartingPositions,
                      mHouseMapMobileCellFades,
                      mFullMapD, mFullMapD,
                      mRobberIndex,
                      mLastRobberIndex,
                      mStartIndex );

    copyAllIntoSubCells();



    // block death of robber on welcome mat
    // (can't even be shot by wife while on welcome mat)
    // ALSO if robber already hit goal, robber cannot die

    if( mRobberIndex != mStartIndex &&
        mRobberIndex != mGoalIndex ) {
        
        // check possible sources of death
        // robber location and four neighbors
        int xDelta[5] = {  0, 
                           -1,  1,  0,  0 };
        int yDelta[5] = {  0,  
                           0,  0,  -1,  1 };

        // tile/mobile at robber location can only kill 
        // robber if it is 'deadly'
        // neighbors can only kill robber if they are 'deadlyAdjacent'
        propertyID deathCondition[5] =
            { deadly, 
              deadlyAdjacent, deadlyAdjacent, deadlyAdjacent, deadlyAdjacent };
    
        int robberX = mRobberIndex % mFullMapD;
        int robberY = mRobberIndex / mFullMapD;
            
        for( int s=0; s<5; s++ ) {
            int x = robberX + xDelta[s];
            int y = robberY + yDelta[s];
            
            if( x >= 0 && x < mFullMapD &&
                y >= 0 && y < mFullMapD ) {

                int deathSourceIndex = y * mFullMapD + x;



                if( isPropertySet( 
                        mHouseMapIDs[ deathSourceIndex ], 
                        mHouseMapCellStates[ deathSourceIndex ], 
                        deathCondition[s] ) ) {
        
                    // robber killed by static object
                    mDead = true;
                    mDeathSourceID = mHouseMapIDs[ deathSourceIndex ];
                    mDeathSourceState = 
                        mHouseMapCellStates[ deathSourceIndex ];
        
                    mRobberState = checkTransition( PLAYER_ID, mRobberState,
                                                    mDeathSourceID,
                                                    mDeathSourceState );
                    mSuccess = 0;
                    processFamilyAndMobilesAtEnd();
                    break;
                    }
                else if( isPropertySet( 
                             mHouseMapMobileIDs[ deathSourceIndex ], 
                             mHouseMapMobileCellStates[ deathSourceIndex ], 
                             deathCondition[s] ) ) {

                    // robber killed by mobile
                    mDead = true;
                    mDeathSourceID = mHouseMapMobileIDs[ deathSourceIndex ];
                    mDeathSourceState = 
                        mHouseMapMobileCellStates[ deathSourceIndex ];

                    mRobberState = checkTransition( PLAYER_ID, mRobberState,
                                                    mDeathSourceID,
                                                    mDeathSourceState );
                    mSuccess = 0;
                    processFamilyAndMobilesAtEnd();
                    break;
                    }
                }
            }
        }
    
    
    
    recomputeVisibility();



    // now trigger transitions based on the new visibility
    
    char *fullMapTilesVisible = new char[ mNumMapSpots ];

    memset( fullMapTilesVisible, false, mNumMapSpots );
    
    for( int y=0; y<HOUSE_D; y++ ) {
        int bigY = y + mSubMapOffsetY;

        for( int x=0; x<HOUSE_D; x++ ) {
            int bigX = x + mSubMapOffsetX;

            int subIndex = y * HOUSE_D + x;
            
            int bigIndex = bigY * mFullMapD + bigX;
            
            fullMapTilesVisible[ bigIndex ] = mTileVisibleMap[ subIndex ];
            }
        }

    
    applyVisibilityTransitions( mHouseMapIDs, mHouseMapCellStates, 
                                mHouseMapMobileIDs, mHouseMapMobileCellStates,
                                mFullMapD, mFullMapD,
                                fullMapTilesVisible );

    delete [] fullMapTilesVisible;



    // now trigger family members that may have seen robber given
    // this new visibility map
    if( ! areMobilesFrozen() ) {
        int numPaths = mFamilyExitPaths.size();
        for( int i=0; i<numPaths; i++ ) {
            int progress = *( mFamilyExitPathProgress.getElement( i ) );
        
            GridPos *path = *( mFamilyExitPaths.getElement( i ) );

            char seenRobber = *( mFamilySeenRobber.getElement( i ) );
            
            if( !seenRobber ) {
                
                // check if seen this step
                int status = *( mFamilyStatus.getElement( i ) );

                if( status == 1 ) {
                    int subIndex = fullToSub( posToIndex( path[progress] ) );
                    
                    if( subIndex != -1 && mTileVisibleMap[ subIndex ] ) {
                        seenRobber = true;
                        *( mFamilySeenRobber.getElement( i ) ) = seenRobber;
                        }
                    }
                }
            }
        }

    
    }






SpriteHandle RobHouseGridDisplay::generateVisibilityShroudSprite(
    unsigned char *inVisibleMap, char *inTargetVisibleMap,
    int inBlowUpFactor,
    unsigned char inBorderFill,
    int inChangeRateReveal,
    int inChangeRateHide,
    int inNumBlurRounds ) {
    
    // decay each frame
    for( int i=0; i<HOUSE_D * HOUSE_D * VIS_BLOWUP * VIS_BLOWUP; i++ ) {
        if( mRobberIndex == mGoalIndex ) {
            // robber hit vault
            // instantly black out, because robber on vault looks weird
            inVisibleMap[i] = 255;
            }
        else if( inTargetVisibleMap[i] ) {
            // wants to move toward visible
            if( inVisibleMap[i] != 0 ) {
                unsigned char oldValue = inVisibleMap[i];

                // revealing new areas happens at a different rate 
                // than shrouding no-longer-seen areas
                inVisibleMap[i] -= 
                    lrint( inChangeRateReveal * frameRateFactor );
                
                // watch for wrap-around!
                if( inVisibleMap[i] > oldValue ) {
                    inVisibleMap[i] = 0;
                    }

                }
            }
        else {
            // wants to move toward invisible
            
            if( inVisibleMap[i] != 255 ) {
                
                unsigned char oldValue = inVisibleMap[i];

                inVisibleMap[i] += lrint( inChangeRateHide * frameRateFactor );
                
                // watch for wrap-around!
                if( inVisibleMap[i] < oldValue ) {
                    inVisibleMap[i] = 255;
                    }
                }
            }
        }


    int blowUpFactor = inBlowUpFactor;
    int blownUpSize = HOUSE_D * VIS_BLOWUP * blowUpFactor;

    double log2size = log( blownUpSize ) / log( 2 );
    
    int next2PowerSize = (int)( ceil( log2size  ) );
    
    int paddedSize = blownUpSize;

    if( next2PowerSize != log2size ) {
        paddedSize = (int)( pow( 2, next2PowerSize ) );
        }
    
    int blowUpBorder = ( paddedSize - blownUpSize ) / 2;
    

    int numBlowupPixels = paddedSize * paddedSize;

    // opt:  do all this processing with uchars instead of doubles
    unsigned char *fullGridChannelsBlownUpAlpha =
        new unsigned char[ numBlowupPixels ];

    int *touchIndices = new int[ numBlowupPixels ];

    int numTouched = 0;

    memset( fullGridChannelsBlownUpAlpha, inBorderFill, numBlowupPixels );

    
    for( int y=0; y<HOUSE_D * VIS_BLOWUP; y++ ) {
        
        for( int x=0; x<HOUSE_D * VIS_BLOWUP; x++ ) {
            
            unsigned char alphaValue = 
                inVisibleMap[ y * HOUSE_D * VIS_BLOWUP + x ];

            for( int blowUpY= y * blowUpFactor; 
                 blowUpY< y * blowUpFactor + blowUpFactor; 
                 blowUpY++ ) {

                int padY = blowUpY + blowUpBorder;

                for( int blowUpX= x * blowUpFactor; 
                     blowUpX< x * blowUpFactor + blowUpFactor; 
                     blowUpX++ ) {
                    
                    int padX = blowUpX + blowUpBorder;

                    int imageIndex = padY * paddedSize + padX;
                    
                    fullGridChannelsBlownUpAlpha[ imageIndex ] = alphaValue;
                    
                    if( padY > 0 && padY < paddedSize - 1
                        &&
                        padX > 0 && padX < paddedSize - 1 ) {
                        
                        // apply blur filter to non-border pixels
                        touchIndices[numTouched] = imageIndex;
                        numTouched++;
                        }
                    else {
                        // set all border pixels to inBorderFill
                        // and don't apply blur filter to those
                        fullGridChannelsBlownUpAlpha[ imageIndex ] = 
                            inBorderFill;
                        }
                    }
                }
            }
        }
    
    
    FastBoxBlurFilter filter2;

    int blurRoundsLeft = inNumBlurRounds;
    
    int blurRoundsBeforePunchThrough = inNumBlurRounds / 2 - 2;
    if( blurRoundsBeforePunchThrough > inNumBlurRounds ) {
        blurRoundsBeforePunchThrough = 0;
        }
    // do first half now before punching through for mobiles (plus tweak
    // of -2, so we do two extra after punch through)
    for( int f=0; f<blurRoundsBeforePunchThrough; f++ ) {
        
        filter2.applySubRegion( fullGridChannelsBlownUpAlpha, 
                                touchIndices,
                                numTouched,
                                paddedSize, paddedSize );
        blurRoundsLeft--;
        }





    // clear shround areas over any visible mobiles or family
    // (punch holes through shround to make sure they're not too obscured)

    for( int y=0; y<HOUSE_D; y++ ) {
        for( int x=0; x<HOUSE_D; x++ ) {
            int tileI = y * HOUSE_D + x;
            
            if( mShroudPunchThroughMap[tileI] < 255 ) {

                // punch a hole in shroud here
                                
                unsigned char newAlpha = mShroudPunchThroughMap[tileI];
                
                int totalBlowupFactor = blowUpFactor * VIS_BLOWUP;

                for( int blowUpY= y * totalBlowupFactor; 
                     blowUpY< y * totalBlowupFactor + totalBlowupFactor; 
                     blowUpY++ ) {

                    int flipY = totalBlowupFactor * HOUSE_D - blowUpY - 1;
                        

                    int padY = flipY + blowUpBorder;
                        

                    for( int blowUpX= x * totalBlowupFactor; 
                         blowUpX< x * totalBlowupFactor + 
                             totalBlowupFactor; 
                         blowUpX++ ) {
                        
                        int padX = blowUpX + blowUpBorder;
                        
                        int imageIndex = padY * paddedSize + padX;
                        
                        if( fullGridChannelsBlownUpAlpha[ imageIndex ]
                            > newAlpha ) {
                            
                            // pull down to more transparent
                            // but never make shroud more opaque
                            fullGridChannelsBlownUpAlpha[ imageIndex ] = 
                                newAlpha;
                            }
                        }
                    }
                }
            }
        }
    
    
    
    // second half of blur rounds, after punching holes for mobiles
    for( int f=0; f<blurRoundsLeft; f++ ) {
        
        filter2.applySubRegion( fullGridChannelsBlownUpAlpha, 
                                touchIndices,
                                numTouched,
                                paddedSize, paddedSize );
        }
    

    

    // clear border again, but leave one extra pixel for map-edge shrouding
    for( int y=0; y<paddedSize; y++ ) {
        for( int x=0; x<blowUpBorder-1; x++ ) {
            fullGridChannelsBlownUpAlpha[ y * paddedSize + x ] = 0;
            }
        for( int x=paddedSize - blowUpBorder + 1; x<paddedSize; x++ ) {
            fullGridChannelsBlownUpAlpha[ y * paddedSize + x ] = 0;
            }
        }
    for( int x=0; x<paddedSize; x++ ) {
        for( int y=0; y<blowUpBorder-1; y++ ) {
            fullGridChannelsBlownUpAlpha[ y * paddedSize + x ] = 0;
            }
        for( int y=paddedSize - blowUpBorder + 1; y<paddedSize; y++ ) {
            fullGridChannelsBlownUpAlpha[ y * paddedSize + x ] = 0;
            }
        }


    SpriteHandle visSprite = 
        fillSpriteAlphaOnly( fullGridChannelsBlownUpAlpha, 
                             paddedSize, paddedSize );
    
    delete [] fullGridChannelsBlownUpAlpha;
    delete [] touchIndices;

    return visSprite;
    }




void RobHouseGridDisplay::setConnectedFadeRecursive(
    float inTargetFadeValue, int inStartTileID,
    int inCurrentIndex, char *inVisitedFlags ) {
    

    for( int n=0; n<4; n++ ) {
        int nIndex = getTileNeighbor( inCurrentIndex, n );
        
        if( nIndex != -1 && 
            fullToSub( nIndex ) != -1 &&
            ! inVisitedFlags[ nIndex ] ) {
            // on visible map
            
            if( mHouseMapCellFades[nIndex] < inTargetFadeValue &&
                isInGroup( inStartTileID, 
                           mHouseMapIDs[nIndex] ) ) {
                
                mHouseMapCellFades[nIndex] = inTargetFadeValue;
                inVisitedFlags[nIndex] = true;
                
                setConnectedFadeRecursive( inTargetFadeValue,
                                           inStartTileID,
                                           nIndex,
                                           inVisitedFlags );
                }
            }
        }
    }






void RobHouseGridDisplay::propagateVisibilityFades( float inFadeRevealStep, 
                                                    float inFadeHideStep ) {
    
    char *beenSteppedDown = new char[ mNumMapSpots ];
    memset( beenSteppedDown, false, mNumMapSpots );

    for( int i=0; i<mNumMapSpots; i++ ) {
        
        float target = mHouseMapCellFadesWorking[i];

        if( mHouseMapCellFades[i] <= target ) {
            // we can safely jump right up to our target value
            // (because the working values are already smoothly adjusted up)
            mHouseMapCellFades[i] = target;
            }
        else {
            // if our target is lower, fade gradually down to it
            // because our propagated value can drop out from underneath
            // us (like if a door closes), and we can't abruptly jump
            // down to our target
            mHouseMapCellFades[i] -= inFadeHideStep;
            beenSteppedDown[i] = true;
            if( mHouseMapCellFades[i] < target ) {
                mHouseMapCellFades[i] = target;
                }
            }
        
        }

    
    char *alreadySteppedUp = new char[ mNumMapSpots ];
    char *alreadySteppedUpMobile = new char[ mNumMapSpots ];
    memset( alreadySteppedUp, false, mNumMapSpots );
    memset( alreadySteppedUpMobile, false, mNumMapSpots );
    
    
    int numScreenTiles = HOUSE_D * HOUSE_D;

    // first pass:
    // propagate visibility to neighbors if non-vision blocking
    for( int i=0; i<numScreenTiles; i++ ) {
        int fullIndex = subToFull( i );
        
        float fade = mHouseMapCellFadesWorking[fullIndex];
        
        if( fade > 0 ) {
            
            if( ! isMapPropertySet( fullIndex, 
                                    visionBlocking ) ) {
            
                // we're not vision blocking, 
                // so make sure neighbors aren't more faded

                for( int n=0; n<8; n++ ) {
                    int nIndex = getTileNeighborEight( fullIndex, n );
                    
                    if( nIndex != -1 && 
                        ( !alreadySteppedUp[ nIndex ] ||
                          !alreadySteppedUpMobile[ nIndex ] ) ) {
                        
                        char neighborMatchFade = false;
                        
                        if( n % 2 == 0 ) {
                            // cardinal NSEW neighbors always match our fade
                            neighborMatchFade = true;
                            }
                        else {
                            // odd neighbors are diagonal
                            // if diagonal neighbor not -1, we know that
                            // adjacent cardinal neighbors are in-bounds too

                            // only pass fade to diagonal neighbors
                            // if adjacent cardinal neighbors are 
                            // not both vision blocking
                            // this prevents seeing through diagonal gaps
                            
                            int nA = n - 1;
                            int nB = (n+1)%8;
                            
                            if( isMapPropertySet( 
                                    getTileNeighborEight( fullIndex, nA ),
                                    visionBlocking ) &&
                                isMapPropertySet( 
                                    getTileNeighborEight( fullIndex, nB ),
                                    visionBlocking ) ) {
                                
                                if( isMapPropertySet( nIndex, 
                                                      visionBlocking ) ) {
                                    // if diagonal neighbor is itself
                                    // vision blocking, then let it show
                                    // through here, because sometimes
                                    // it is actually visible through
                                    // the gap, so make it always visible
                                    // instead of having it jump in and out
                                    // as robber line of sight changes
                                    neighborMatchFade = true;
                                    }
                                }
                            else {
                                neighborMatchFade = true;
                                }
                            }
                        
                        // we're jumping up above our smoothly-faded
                        // target
                        // So we need to smoothly-fade this jump ourselves

                        if( neighborMatchFade ) {
                            if( !alreadySteppedUp[nIndex] &&
                                mHouseMapCellFades[nIndex] < fade ) {
                                mHouseMapCellFades[nIndex] += 
                                    inFadeRevealStep;
                                if( beenSteppedDown[nIndex ] ) {
                                    // counter-act the stepping down
                                    // that already happened above
                                    mHouseMapCellFades[nIndex] += 
                                        inFadeHideStep;
                                    }
                                if( mHouseMapCellFades[nIndex] > fade ) {
                                    mHouseMapCellFades[nIndex] = fade;
                                    }
                                alreadySteppedUp[nIndex] = true;
                                }
                            if( !alreadySteppedUpMobile[nIndex] &&
                                mHouseMapMobileCellFades[nIndex] < fade ) {
                                mHouseMapMobileCellFades[nIndex] += 
                                    inFadeRevealStep;
                                if( mHouseMapMobileCellFades[nIndex] > fade ) {
                                    mHouseMapMobileCellFades[nIndex] = fade;
                                    }
                                alreadySteppedUpMobile[nIndex] = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    

    // second pass:
    // propagate through chains of connected tiles
    char *visitedFlags = new char[ mNumMapSpots];

    for( int i=0; i<numScreenTiles; i++ ) {
        int fullIndex = subToFull( i );
        
        float newFade = mHouseMapCellFades[fullIndex];
        if( newFade > 0 ) {
            // tile is visible or became visible because of immediate,
            // non-blocking neighbors
        
            // pass its visibility on to any connected neighbors
            int houseMapTile = mHouseMapIDs[fullIndex];
            
            char isConnectable = 
                ( getNumOrientations( houseMapTile, 0 ) == 16 );

            if( isConnectable ) {
                
                memset( visitedFlags, false, mNumMapSpots );

                visitedFlags[fullIndex] = true;
                
                setConnectedFadeRecursive( newFade, houseMapTile,
                                           fullIndex, visitedFlags );
                }
            }
        }

    delete [] visitedFlags;
    delete [] alreadySteppedUp;
    delete [] alreadySteppedUpMobile;
    delete [] beenSteppedDown;
    }





    

void RobHouseGridDisplay::draw() {
    HouseGridDisplay::draw();

    
    // apply same scissor region to visibility overlay
    enableScissor( -HOUSE_D * mTileRadius,
                   -HOUSE_D * mTileRadius,
                   2 * ( HOUSE_D * mTileRadius ),
                   2 * ( HOUSE_D * mTileRadius ) );
    

    

    int blowUpFactor = 2;
    
    int mainShroudRevealRate =  20;
    int mainShroudHideRate = 5;

    int slipShroudRevealRate =  60;
    int slipShroudHideRate = 1;


    // update shroud punch-throughs to move toward targets
    // fades in and out at same rate as main shround
    for( int i=0; i<HOUSE_D * HOUSE_D; i++ ) {
        if( mRobberIndex == mGoalIndex ) {
            // robber hit vault
            // instantly black out, because robber on vault looks weird
            mShroudPunchThroughMap[i] = 255;
            }
        else if( mTargetShroudPunchThroughMap[i] ) {
            // wants to move toward visible
            if( mShroudPunchThroughMap[i] != 0 ) {
                unsigned char oldValue = mShroudPunchThroughMap[i];

                // revealing new areas happens at a different rate 
                // than shrouding no-longer-seen areas
                mShroudPunchThroughMap[i] -= 
                    lrint( mainShroudRevealRate * frameRateFactor );
                
                // watch for wrap-around!
                if( mShroudPunchThroughMap[i] > oldValue ) {
                    mShroudPunchThroughMap[i] = 0;
                    }

                }
            }
        else {
            // wants to move toward invisible
            
            if( mShroudPunchThroughMap[i] != 255 ) {
                
                unsigned char oldValue = mShroudPunchThroughMap[i];

                mShroudPunchThroughMap[i] += 
                    lrint( mainShroudHideRate * frameRateFactor );
                
                // watch for wrap-around!
                if( mShroudPunchThroughMap[i] < oldValue ) {
                    mShroudPunchThroughMap[i] = 255;
                    }
                }
            }
        }



    SpriteHandle visSprite = 
        generateVisibilityShroudSprite( mVisibleMap, mTargetVisibleMap,
                                        blowUpFactor, 255, 
                                        mainShroudRevealRate, 
                                        mainShroudHideRate,
                                        10 );
    
    // slip changes much faster to avoid reducing fluidity of main shroud
    SpriteHandle visSlipSprite = 
        generateVisibilityShroudSprite( mVisibleUnderSlipMap, 
                                        mTargetVisibleUnderSlipMap,
                                        blowUpFactor, 0,
                                        slipShroudRevealRate,
                                        slipShroudHideRate,
                                        6 );
    

    // set tile fade at same rate as main vis shroud sprite so that 
    // non-visible tiles disappear (and are not peeking out) under shroud
    int numScreenTiles = HOUSE_D * HOUSE_D;
    float fadeRevealStep = mainShroudRevealRate * frameRateFactor / 255.0f;
    
    // connected tiles show seams when given a slower reveal fade-in
    // to avoid this, make them pop in at the faster underslip reveal rate
    float fadeRevealStepConnectedTiles = 
        slipShroudRevealRate * frameRateFactor / 255.0f;

    // hide everything slowly as it is consumed by shroud.  This looks great.
    float fadeHideStep = slipShroudHideRate * frameRateFactor / 255.0f;
    
    for( int i=0; i<numScreenTiles; i++ ) {
        int fullIndex = subToFull( i );

        // if block on, all tiles should move toward visible
        char shouldBeVisible = mBlockVisibilityFading || mTileVisibleMap[i];
        
        char isTileConnected = 
            ( getNumOrientations( mHouseMapIDs[fullIndex], 0 ) == 16 );

        if( shouldBeVisible && 
            mHouseMapCellFadesWorking[ fullIndex ] != 1 ) {
            
            if( isTileConnected ) {
                mHouseMapCellFadesWorking[ fullIndex ] += 
                    fadeRevealStepConnectedTiles;
                }
            else {
                mHouseMapCellFadesWorking[ fullIndex ] += fadeRevealStep;
                }
            
            if( mHouseMapCellFadesWorking[ fullIndex ] > 1 ) {
                mHouseMapCellFadesWorking[ fullIndex ] =  1;
                }
            }
        else if( ! shouldBeVisible && 
            mHouseMapCellFadesWorking[ fullIndex ] != 0 ) {
            
            mHouseMapCellFadesWorking[ fullIndex ] -= fadeHideStep;

            if( mHouseMapCellFadesWorking[ fullIndex ] < 0 ) {
                mHouseMapCellFadesWorking[ fullIndex ] =  0;
                }
            }

        if( shouldBeVisible && 
            mHouseMapMobileCellFades[ fullIndex ] != 1 ) {
            
            mHouseMapMobileCellFades[ fullIndex ] += fadeRevealStep;

            if( mHouseMapMobileCellFades[ fullIndex ] > 1 ) {
                mHouseMapMobileCellFades[ fullIndex ] =  1;
                }
            }
        else if( ! shouldBeVisible && 
            mHouseMapMobileCellFades[ fullIndex ] != 0 ) {
            
            mHouseMapMobileCellFades[ fullIndex ] -= fadeHideStep;

            if( mHouseMapMobileCellFades[ fullIndex ] < 0 ) {
                mHouseMapMobileCellFades[ fullIndex ] =  0;
                }
            }

        }
    
    propagateVisibilityFades( fadeRevealStep, fadeHideStep );

        
        


    doublePair spritePos = { 0, 0 };
    
    setDrawColor( 0, 0, 0, 1 );

    toggleLinearMagFilter( true );

    drawSprite( visSprite, spritePos, 
                1.0 * 2 * mTileRadius / ( blowUpFactor * VIS_BLOWUP ) );
    drawSprite( visSlipSprite, spritePos, 
                1.0 * 2 * mTileRadius / ( blowUpFactor * VIS_BLOWUP ) );
    
        
    toggleLinearMagFilter( false );
    
    freeSprite( visSprite );
    freeSprite( visSlipSprite );


    disableScissor();


    if( mLeaveDisplayCanBeShown && mRobberIndex == mStartIndex ) {
        doublePair startPos = getTilePosFull( mStartIndex );
        
        startPos.x -= 1;

        setDrawColor( 1, 1, 1, 1 );
        
        drawSprite( mLeaveSprite, startPos, 
                    1.0 / 16.0 );
        
        startPos.x -= 0.5;

        mainFont->drawString( translate( "leave" ), startPos, alignRight );;

        }
    
    }



char RobHouseGridDisplay::getAboutToLeave() {
    return ( mRobberIndex == mStartIndex );
    }




void RobHouseGridDisplay::applyCurrentTool( int inTargetFullIndex ) {
    
    if( mToolTargetFullIndices.getElementIndex( inTargetFullIndex ) != -1 ) {
        // this is an allowed target spot for this tool
        
        applyToolTransition( mHouseMapIDs, mHouseMapCellStates, 
                             mHouseMapMobileIDs, mHouseMapMobileCellStates,
                             mFullMapD, mFullMapD,
                             mCurrentTool, inTargetFullIndex );
        }
    else {
        mForbiddenMoveHappened = true;
        }
    
    // tool use triggers a step, but not for mobiles
    freezeMobileObjects( true );
    applyTransitionsAndProcess();
    freezeMobileObjects( false );
    
    stopUsingTool();
    mToolJustUsed = true;
    }




void RobHouseGridDisplay::pointerOver( float inX, float inY ) {
    
    int overIndex = getTileIndex( inX, inY );
    

    // don't show tool tips for invisible tiles (shrouded)
    // or during blackout after robber hits goal
    if( overIndex != -1 && 
        ( mTileVisibleMap[ overIndex ] || mForceAllTileToolTips ) 
        &&
        mRobberIndex != mGoalIndex ) {

        // base behavior (display tool tip)
        HouseGridDisplay::pointerOver( inX, inY );
        
        // but override so that highlight is never shown
        mHighlightIndex = -1;
        }
    else if( mPointerInside ) {
        // pointer just left visible region
        setToolTip( NULL );
        
        mPointerInside = false;
        }
    
        
        
    }



// override so that they do nothing but display tool tips
void RobHouseGridDisplay::pointerMove( float inX, float inY ) {
    pointerOver( inX, inY );
    }


void RobHouseGridDisplay::pointerDown( float inX, float inY ) {
    pointerDrag( inX, inY );
    }



void RobHouseGridDisplay::pointerDrag( float inX, float inY ) {
    pointerOver( inX, inY );
    
    setPickedTargetHighlight( -1 );
    
    if( mCurrentTool != -1 ) {
        // see if this is in our target region
        int index = subToFull( getTileIndex( inX, inY ) );

        if( mToolTargetFullIndices.getElementIndex( index ) != -1 ) {
            // hit
            setPickedTargetHighlight( index );
            }
        }
    }


void RobHouseGridDisplay::pointerUp( float inX, float inY ) {
    pointerOver( inX, inY );

    if( mCurrentTool != -1 ) {
        // see if this is in our target region
        int index = subToFull( getTileIndex( inX, inY ) );

        if( mToolTargetFullIndices.getElementIndex( index ) != -1 ) {
            // hit

            mMoveList.push_back( 
                autoSprintf( "t%d@%d", mCurrentTool, index ) );
            
            
            applyCurrentTool( index );
            
            fireActionPerformed( this );
            }
        }
    

    }



void RobHouseGridDisplay::keyDown( unsigned char inASCII ) {    
    if( mSafeMoveMode && mSafeMoveProposed ) {
        
        if( inASCII == 13 ) {
            mSafeMoveConfirmed = true;
            // apply move again, now that we have confirmation
            moveRobber( mSafeMoveIndex );

            // clear if our waiting-for-confirmation tip is still showing 
            clearToolTip( translate( "safeMoveTip" ) );

            // no action fired from this move because it isn't triggered
            // by a specialKeyDown event in HouseGridDisplay
            // (that event already happened, before confirmation)
            fireActionPerformed( this );
            }
        }
    
    HouseGridDisplay::keyDown( inASCII );
    }




void RobHouseGridDisplay::moveRobber( int inNewIndex ) {
    if( mSuccess != 0 || mDead ) {
        // can't move anymore
        return;
        }

    if( mCurrentTool != -1 ) {
        // turn tool off when robber moves
        stopUsingTool();
        mToolJustUsed = false;
        }


    if( mSafeMoveMode ) {
        if( mSafeMoveProposed && mSafeMoveConfirmed &&
            inNewIndex == mSafeMoveIndex ) {
            // this call is a confirmation

            // go ahead and move, below
            mSafeMoveConfirmed = false;
            mSafeMoveProposed = false;
            mSafeMoveIndex = -1;

            clearSpecialHighlights();
            }
        else {
            // this is a new proposal

            mSafeMoveConfirmed = false;
            mSafeMoveProposed = true;
            mSafeMoveIndex = inNewIndex;
            clearSpecialHighlights();
            
            // add highlight for this proposal
            doublePair robberPos = getTilePosFull( mRobberIndex );
            doublePair movePos = getTilePosFull( mSafeMoveIndex );
        
            double dX = movePos.x - robberPos.x;
            double dY = movePos.y - robberPos.y;
            
            SpriteHandle moveSprite;
            
            if( dX < 0 ) {
                moveSprite = mSafeLeftSprite;
                }
            else if( dX > 0 ) {
                moveSprite = mSafeRightSprite;
                }
            else if( dY < 0 ) {
                moveSprite = mSafeDownSprite;
                }
            else if( dY > 0 ) {
                moveSprite = mSafeUpSprite;
                }

            
            addSpecialHighlight( mSafeMoveIndex, moveSprite );
            setToolTip( translate( "safeMoveTip" ) );
            
            // wait for confirmation
            return;
            }
        }
    
    

    HouseGridDisplay::moveRobber( inNewIndex );

    if( mSuccess == 2 ) {
        // left, don't apply transitions
        return;
        }

    // did robber step onto killed wife?
    if( getWifeKilled() ) {
        if( isPropertySet( mHouseMapIDs[ mRobberIndex ], 0, wife ) ) {
            mRobberStoleFromWife = true;
            setWifeMoney( 0 );
            }
        }

    applyTransitionsAndProcess();
    
    // a move!
    mMoveList.push_back( autoSprintf( "m%d", mRobberIndex ) );
    
    if( mRobberIndex == mGoalIndex ) {
        mSuccess = 1;
        processFamilyAndMobilesAtEnd();

        fireActionPerformed( this );
        }
    }



void RobHouseGridDisplay::robberTriedToLeave() {
    if( mSuccess == 0 && ! mDead ) {
        
        mSuccess = 2;
        processFamilyAndMobilesAtEnd();

        mMoveList.push_back( stringDuplicate( "L" ) );
        
        fireActionPerformed( this );
        }
    // else ignore it, and don't allow robbery to leave twice, 
    // even if player is mashing keys
    }



void RobHouseGridDisplay::setVisibleOffset( int inXOffset, int inYOffset ) {
    int xExtra = inXOffset - mSubMapOffsetX;
    int yExtra = inYOffset - mSubMapOffsetY;
    
    if( xExtra != 0 || yExtra != 0 ) {        
        // slide values in visibility maps
        unsigned char *mNewVisibleMap = new unsigned char[ 
            VIS_BLOWUP * HOUSE_D * VIS_BLOWUP * HOUSE_D ];

        unsigned char *mNewVisibleUnderSlipMap = new unsigned char[ 
            VIS_BLOWUP * HOUSE_D * VIS_BLOWUP * HOUSE_D ];

        // leave black in excess area
        memset( mNewVisibleMap, 255, 
                VIS_BLOWUP * HOUSE_D * VIS_BLOWUP * HOUSE_D );
        memset( mNewVisibleUnderSlipMap, 255, 
                VIS_BLOWUP * HOUSE_D * VIS_BLOWUP * HOUSE_D );

        int visXDelta = -xExtra * VIS_BLOWUP;
        int visYDelta = yExtra * VIS_BLOWUP;
        
        for( int y=0; y<VIS_BLOWUP * HOUSE_D; y++ ) {
            int destY = y + visYDelta;
            
            if( destY >= 0 && destY < VIS_BLOWUP * HOUSE_D ) {
                
                for( int x=0; x<VIS_BLOWUP * HOUSE_D; x++ ) {
                    
                    int destX = x + visXDelta;
                    
                    if( destX >= 0 && destX < VIS_BLOWUP * HOUSE_D ) {

                        int i = y * VIS_BLOWUP * HOUSE_D + x;
                        int destI = destY * VIS_BLOWUP * HOUSE_D + destX;
                        
                            
                        mNewVisibleMap[ destI ]
                            = mVisibleMap[ i ];
                        mNewVisibleUnderSlipMap[ destI ]
                            = mVisibleUnderSlipMap[ i ];
                        }
                    }
                }
            }
        
        // now replace old with new
        memcpy( mVisibleMap, mNewVisibleMap, 
                VIS_BLOWUP * HOUSE_D * VIS_BLOWUP * HOUSE_D );
        memcpy( mVisibleUnderSlipMap, mNewVisibleUnderSlipMap, 
                VIS_BLOWUP * HOUSE_D * VIS_BLOWUP * HOUSE_D );
        delete [] mNewVisibleMap;
        delete [] mNewVisibleUnderSlipMap;




        // same for punch through map
        // slide values
        unsigned char *mNewShroudPunchThroughMap = new unsigned char[ 
            HOUSE_D * HOUSE_D ];

        // leave black in excess area
        memset( mNewShroudPunchThroughMap, 255, HOUSE_D * HOUSE_D );
                
        for( int y=0; y<HOUSE_D; y++ ) {
            int destY = y - yExtra;
            
            if( destY >= 0 && destY < HOUSE_D ) {
                
                for( int x=0; x<HOUSE_D; x++ ) {
                    
                    int destX = x - xExtra;
                    
                    if( destX >= 0 && destX < HOUSE_D ) {

                        int i = y * HOUSE_D + x;
                        int destI = destY * HOUSE_D + destX;
                        
                            
                        mNewShroudPunchThroughMap[ destI ]
                            = mShroudPunchThroughMap[ i ];
                        }
                    }
                }
            }
        
        // now replace old with new
        memcpy( mShroudPunchThroughMap, mNewShroudPunchThroughMap, 
                HOUSE_D * HOUSE_D );
        delete [] mNewShroudPunchThroughMap;
        }
    
    HouseGridDisplay::setVisibleOffset( inXOffset, inYOffset );
    
    
    if( mHouseMapCellFadesWorking == NULL ) {
        // skip the fade shift below, because our house
        // map hasn't been fully set yet, and all fades will 
        // be set to 0 by the end of RobHouseGridDisplay::setHouseMap anyway
        return;
        }
    
    // set tile fade to 0 for all parts of map that are outside our 
    // screen window 
    for( int y=0; y<mSubMapOffsetY; y++ ) {
        for( int x=0; x<mFullMapD; x++ ) {
            mHouseMapCellFades[y * mFullMapD + x] = 0;
            mHouseMapCellFadesWorking[y * mFullMapD + x] = 0;
            mHouseMapMobileCellFades[y * mFullMapD + x] = 0;
            }
        }
    for( int y=mSubMapOffsetY+HOUSE_D; y<mFullMapD; y++ ) {
        for( int x=0; x<mFullMapD; x++ ) {
            mHouseMapCellFades[y * mFullMapD + x] = 0;
            mHouseMapCellFadesWorking[y * mFullMapD + x] = 0;
            mHouseMapMobileCellFades[y * mFullMapD + x] = 0;
            }
        }
    for( int x=0; x<mSubMapOffsetX; x++ ) {
        for( int y=0; y<mFullMapD; y++ ) {
            mHouseMapCellFades[y * mFullMapD + x] = 0;
            mHouseMapCellFadesWorking[y * mFullMapD + x] = 0;
            mHouseMapMobileCellFades[y * mFullMapD + x] = 0;
            }
        }
    for( int x =mSubMapOffsetX+HOUSE_D; x<mFullMapD; x++ ) {
        for( int y=0; y<mFullMapD; y++ ) {
            mHouseMapCellFades[y * mFullMapD + x] = 0;
            mHouseMapCellFadesWorking[y * mFullMapD + x] = 0;
            mHouseMapMobileCellFades[y * mFullMapD + x] = 0;
            }
        }

    
    }




void RobHouseGridDisplay::recomputeVisibility() {

    // clear
    memset( mTileVisibleMap, 0, HOUSE_D * HOUSE_D );


    int robSubIndex = fullToSub( mRobberIndex );
    
    if( robSubIndex == -1 ) {
        // robber out of bounds
        // nothing visible
        for( int i=0; i<HOUSE_D * HOUSE_D * VIS_BLOWUP * VIS_BLOWUP; i++ ) {
            mTargetVisibleMap[i] = false;
            mTargetVisibleUnderSlipMap[i] = false;
            }
        for( int i=0; i<HOUSE_D * HOUSE_D; i++ ) {
            mTargetShroudPunchThroughMap[i] = false;
            }
        return;
        }

    recomputeVisibilityInt();    
    }







typedef struct intPair {
        int x, y;
    } intPair;


// case type 1:
// XB
// AX

// case type 2:
// AX
// XB


typedef struct cornerLeak {
        intPair a, b;
        int caseType;
    } cornerLeak;




void RobHouseGridDisplay::recomputeVisibilityInt() {
    int robSubIndex = fullToSub( mRobberIndex );

    int robberX = robSubIndex % HOUSE_D;
    int robberY = robSubIndex / HOUSE_D;


    // is robber sandwitched between walls on opposite sides?
    // if so, this makes shroud too dark around robber, so we
    // need to compensate
    char robberBetweenWalls = false;
    
    // robber NEVER at edge of screen
    if( isSubMapPropertySet( robSubIndex - 1, visionBlocking ) &&
        isSubMapPropertySet( robSubIndex + 1, visionBlocking ) ) {
        robberBetweenWalls = true;
        }
    if( isSubMapPropertySet( robSubIndex - HOUSE_D, visionBlocking ) &&
        isSubMapPropertySet( robSubIndex + HOUSE_D, visionBlocking ) ) {
        robberBetweenWalls = true;
        }


    char *blockingMap = new char[ HOUSE_D * HOUSE_D ];    

    memset( blockingMap, 0, HOUSE_D * HOUSE_D );

    for( int i=0; i<HOUSE_D * HOUSE_D; i++ ) {
        if( isSubMapPropertySet( i, visionBlocking ) ) {
            
            blockingMap[ i ] = 1;
            }
        }

    // now patch-up blocking map to fix diagonal corners that look like this:
    //
    //  OOOOOO
    //  OOOXOO
    //  OOXOOO
    //  OOOOOO
    //
    // And tend to let vision through.
    //
    // We have to spots to consider patching, A and B:
    //
    //  OOOOOO
    //  OOAXOO
    //  OOXBOO
    //  OOOOOO
    //
    // We can patch the "back side" safely, because the robber
    // shouldn't be able to see it anyway (if corner blocks vision).
    //
    // Joshua Collins points out a smart, simple algorithm:
    //
    // Consider four regions around the gap:
    //
    //  AAACCC
    //  AAAXCC
    //  DDXBBB
    //  DDDBBB
    //
    // If robber is standing in region A, patch B side of gap.
    // If robber is standing in region B, patch A side of gap.
    //
    // If robber is standing in C or D, patch nothing, because robber cannot
    // see through the gap anyway.
    

    SimpleVector<cornerLeak> leaks;
    
    for( int y=0; y<HOUSE_D-1; y++ ) {
        for( int x=0; x<HOUSE_D-1; x++ ) {
            
            // case type 1
            if( blockingMap[ y * HOUSE_D + x ] &&
                blockingMap[ (y+1) * HOUSE_D + x + 1 ] &&
                ! blockingMap[ (y+1) * HOUSE_D + x ] &&
                ! blockingMap[ y * HOUSE_D + x + 1 ] ) {
                
                cornerLeak l = { {x, y+1}, {x+1, y},
                                 1 };
                leaks.push_back( l );
                }
            // case type 2
            if( !blockingMap[ y * HOUSE_D + x ] &&
                !blockingMap[ (y+1) * HOUSE_D + x + 1 ] &&
                blockingMap[ (y+1) * HOUSE_D + x ] &&
                blockingMap[ y * HOUSE_D + x + 1 ] ) {
                
                cornerLeak l = { {x, y}, {x+1, y+1},
                                 2 };
                leaks.push_back( l );
                }
            }
        }
    
    int numLeaks = leaks.size();
    
    for( int i=0; i<numLeaks; i++ ) {
        
        cornerLeak l = *( leaks.getElement( i ) );
        

        if( l.caseType == 1 ) {
            if( robberX <= l.a.x && robberY >= l.a.y ) {
                // robber in A quadrant, patch B side
                blockingMap[ l.b.y * HOUSE_D + l.b.x ] =  true;
                }
            else if( robberX >= l.b.x && robberY <= l.b.y ) {
                // robber in B quadrant, patch A side
                blockingMap[ l.a.y * HOUSE_D + l.a.x ] =  true;
                }
            // else robber can't see through gap, patch nothing
            }
        else if( l.caseType == 2 ) {
            if( robberX <= l.a.x && robberY <= l.a.y ) {
                // robber in A quadrant, patch B side
                blockingMap[ l.b.y * HOUSE_D + l.b.x ] =  true;
                }
            else if( robberX >= l.b.x && robberY >= l.b.y ) {
                // robber in B quadrant, patch A side
                blockingMap[ l.a.y * HOUSE_D + l.a.x ] =  true;
                }
            // else robber can't see through gap, patch nothing
            }
        }
    


    
    // in middle of blown up tile
    int robberBlowupX = robberX * VIS_BLOWUP + VIS_BLOWUP / 2;
    int robberBlowupY = robberY * VIS_BLOWUP + VIS_BLOWUP / 2;
    

    // make sure there is room for our high res int coordinates, for
    // the farthest subspace tile, in 32-bitint space
    // also room for taking the square of coordinates (for measuring
    // distance)
    // 32767 is small enough to be squared and doubled without exceeding
    // 32-bit INT_MAX
    int highResFactor = 32767 / (VIS_BLOWUP * HOUSE_D );
    


    // match number of steps per tile in old, float implementation
    int numStepsFactor = 2879 / 8;
    // tweak
    numStepsFactor -= 3;
    

    int robberHighY = 
        robberBlowupY * highResFactor;
    int robberHighX = 
        robberBlowupX * highResFactor;


    int backToMapFactor = highResFactor * VIS_BLOWUP;
    

    // how many sub squares are visible on each tile?
    unsigned char *hitCountMap = new unsigned char[ HOUSE_D * HOUSE_D ];
    memset( hitCountMap, 0, HOUSE_D * HOUSE_D );
    
    // mark wall sub-cells to be tweeked to visible later
    // (do it later so as not to affect the underslip)
    SimpleVector<int> tweakToVisibleLater;
    

    int i = 0;
    
    for( int y=0; y<HOUSE_D * VIS_BLOWUP; y++ ) {

        int flipY = HOUSE_D * VIS_BLOWUP - y - 1;


        int mapY = flipY / VIS_BLOWUP;
        int highY = flipY * highResFactor + highResFactor / 2;
        int dHighY = highY - robberHighY;

        for( int x=0; x<HOUSE_D * VIS_BLOWUP; x++ ) {
            int mapX = x / VIS_BLOWUP;
            int highX = x * highResFactor + highResFactor / 2;
            int dHighX =  highX - robberHighX;

            int mapI = mapY * HOUSE_D + mapX;
            
            int distHigh = 
                (int)( sqrt( dHighY * dHighY + dHighX * dHighX ) );
                            
            int numSteps = distHigh / numStepsFactor;

            
            // next:  take this number of steps along high-res
            // line between robber and the point that we're checking
            

            int selfBlockingSteps = 0;

            char hit = false;
            
            // skip last few steps so that we show borders of visible walls

            for( int s=1; s<numSteps - 1; s++ ) {

                int stepHighX = robberHighX + ( dHighX * s ) / numSteps;
                int stepHighY = robberHighY + ( dHighY * s ) / numSteps;

                int stepTileX = stepHighX / backToMapFactor;
                int stepTileY = stepHighY / backToMapFactor;

                int stepI = stepTileY * HOUSE_D + stepTileX;
                
                // avoid self-blocking on first steps into self

                if( selfBlockingSteps < 2 && stepI == mapI ) {
                    // blocked by self for few steps
                    // don't count this as vision blocking
                    selfBlockingSteps ++;
                    }
                else if( blockingMap[ stepI ] ) {
                    hit = true;
                    break;
                    }
                }
            
            if( hit ) {
                mTargetVisibleMap[i] = false;
                }
            else {
                mTargetVisibleMap[i] = true;
                hitCountMap[ mapI ] ++;
                
                // at least one sub-area of tile is visible
                
                // If tile is marked as blocking, it may be a "fill-in"
                // blocking tile on the back side of a diagonal gap.
                // In that case, don't mark whatever is standing there
                // as visible.

                // BUT, make sure that true vision blocking tiles 
                // (like walls) are marked as visible if at least part
                // of them can be seen (otherwise, we can never mouse over 
                // walls and see tool tips, even if we are standing right
                // next to them).
                char isVisionBlocking = 
                    isSubMapPropertySet( mapI, visionBlocking );
                
                if( ! blockingMap[ mapI ] || 
                    isVisionBlocking ) {
                    mTileVisibleMap[ mapI ] = true;
                    }
                if( blockingMap[mapI] ) {
                    
                    // only care about this if robber sandwitched
                    char robberNextToThisWall = false;
                    
                    
                    if( robberBetweenWalls && isVisionBlocking ) {    
                        if( mapY - robberY <= 1 
                            &&
                            mapY - robberY >= -1 
                            &&
                            mapX - robberX <= 1 
                            &&
                            mapX - robberX >= -1 ) {
                            robberNextToThisWall = true;
                            }
                        }
                    
                        
                    mTargetVisibleMap[i] = false;
                    hitCountMap[ mapI ] --;
                    if( robberNextToThisWall ) {
                        // leave walls adjacent to robber partly unshrouded
                        // but not in underslip, so apply later
                        tweakToVisibleLater.push_back( i );
                        }
                    else if( isSubMapPropertySet( mapI, thinShroud ) ) {
                        // same for thin shroud walls
                        tweakToVisibleLater.push_back( i );
                        }
                    
                    }
                
                    
                }
            
            i++;
            }
        }
    


    // make sure there are no orphaned visible tiles that aren't
    // connected to the robber by a contiguous, non-vision blocking visible
    // blob
    // (this also deals will walls that are visible through diagonal gaps)
    char *reachableMap = new char[ HOUSE_D * HOUSE_D ];

    memset( reachableMap, 0, HOUSE_D * HOUSE_D );
    reachableMap[ robSubIndex ] = true;
    SimpleVector<int> frontier;
    frontier.push_back( robSubIndex );
    
    while( frontier.size() > 0 ) {
        
        int nextI = *( frontier.getElement( frontier.size() - 1 ) );
        frontier.deleteElement( frontier.size() - 1 );

        int nextY = nextI / HOUSE_D;
        int nextX = nextI % HOUSE_D;
        
        if( nextY > 0 ) {
            int nI = nextI - HOUSE_D;
            if( !reachableMap[nI] ) {
                reachableMap[nI] = true;
                
                if( mTileVisibleMap[nI] &&
                    !blockingMap[nI] &&
                    !isSubMapPropertySet( nI, visionBlocking ) ) {
                    
                    frontier.push_back( nI );
                    }
                }
            }
        if( nextY < HOUSE_D - 1 ) {
            int nI = nextI + HOUSE_D;
            if( !reachableMap[nI] ) {
                reachableMap[nI] = true;
                
                if( mTileVisibleMap[nI] &&
                    !blockingMap[nI] &&
                    !isSubMapPropertySet( nI, visionBlocking ) ) {
                    
                    frontier.push_back( nI );
                    }
                }
            }
        if( nextX > 0 ) {
            int nI = nextI - 1;
            if( !reachableMap[nI] ) {
                reachableMap[nI] = true;
                
                if( mTileVisibleMap[nI] &&
                    !blockingMap[nI] &&
                    !isSubMapPropertySet( nI, visionBlocking ) ) {
                    
                    frontier.push_back( nI );
                    }
                }
            }
        if( nextX < HOUSE_D - 1 ) {
            int nI = nextI + 1;
            if( !reachableMap[nI] ) {
                reachableMap[nI] = true;
                
                if( mTileVisibleMap[nI] &&
                    !blockingMap[nI] &&
                    !isSubMapPropertySet( nI, visionBlocking ) ) {
                    
                    frontier.push_back( nI );
                    }
                }
            }
        }
    
    for( int i=0; i< HOUSE_D * HOUSE_D; i++ ) {
        if( mTileVisibleMap[i] && ! reachableMap[i] ) {
            
            mTileVisibleMap[i] = false;
           

            // blacken coresponding region of vis overlay
            int mapY = i / HOUSE_D;
            int mapX = i % HOUSE_D;

            // flip
            int visY = (HOUSE_D - mapY - 1) * VIS_BLOWUP;
            int visX = mapX * VIS_BLOWUP;
            
            for( int vY=visY; vY < visY + VIS_BLOWUP; vY ++ ) {
                
                for( int vX=visX; vX < visX + VIS_BLOWUP; vX ++ ) {
                    int vI = vY * HOUSE_D * VIS_BLOWUP + vX;
                    mTargetVisibleMap[vI] = false;
                    }
                }
            }
        }
    delete [] reachableMap;





    // make sure there are no orphaned visible sub-squares that aren't
    // connected to the robber by a contiguous visible blob
    reachableMap = new char[ HOUSE_D * HOUSE_D * VIS_BLOWUP * VIS_BLOWUP ];

    memset( reachableMap, 0, HOUSE_D * HOUSE_D * VIS_BLOWUP * VIS_BLOWUP );

    int robberVisY = (HOUSE_D - robberY) * VIS_BLOWUP - 1;
    int robberVisX = robberX * VIS_BLOWUP;
    int robberVisI = robberVisY * HOUSE_D * VIS_BLOWUP + robberVisX;
    
    reachableMap[ robberVisI ] = true;
    // frontier empty from last loop
    frontier.push_back( robberVisI );
    
    while( frontier.size() > 0 ) {
        
        int nextI = *( frontier.getElement( frontier.size() - 1 ) );
        frontier.deleteElement( frontier.size() - 1 );

        int nextY = nextI / (HOUSE_D * VIS_BLOWUP);
        int nextX = nextI % (HOUSE_D * VIS_BLOWUP);
        
        if( nextY > 0 ) {
            int nI = nextI - (HOUSE_D * VIS_BLOWUP);
            if( !reachableMap[nI] ) {
                reachableMap[nI] = true;
                
                if( mTargetVisibleMap[nI] ) {
                    
                    frontier.push_back( nI );
                    }
                }
            }
        if( nextY < HOUSE_D * VIS_BLOWUP - 1 ) {
            int nI = nextI + (HOUSE_D * VIS_BLOWUP);
            if( !reachableMap[nI] ) {
                reachableMap[nI] = true;
                
                if( mTargetVisibleMap[nI] ) {
                    
                    frontier.push_back( nI );
                    }
                }
            }
        if( nextX > 0 ) {
            int nI = nextI - 1;
            if( !reachableMap[nI] ) {
                reachableMap[nI] = true;
                
                if( mTargetVisibleMap[nI] ) {
                    
                    frontier.push_back( nI );
                    }
                }
            }
        if( nextX < HOUSE_D * VIS_BLOWUP - 1 ) {
            int nI = nextI + 1;
            if( !reachableMap[nI] ) {
                reachableMap[nI] = true;
                
                if( mTargetVisibleMap[nI] ) {
                    
                    frontier.push_back( nI );
                    }
                }
            }
        }



    

    // process visibility map to remove visible "island" sub-squares 
    // that aren'tr eachable from robber through visible squares

    int visMapLimit = HOUSE_D * VIS_BLOWUP;
    
    for( int y=0; y<visMapLimit; y++ ) {

        for( int x=0; x<visMapLimit; x++ ) {
            int i = y * visMapLimit + x;
            
            if( mTargetVisibleMap[i] && ! reachableMap[i] ) {

                // a visibility island
                mTargetVisibleMap[i] = false;
                    
                int flipY = HOUSE_D * VIS_BLOWUP - y - 1;
                
                int mapY = flipY / VIS_BLOWUP;
                int mapX = x / VIS_BLOWUP;
                
                int mapI = mapY * HOUSE_D + mapX;
                
                if( mTileVisibleMap[mapI] ) {
                    
                    
                    hitCountMap[mapI] --;
                    
                    if( hitCountMap[ mapI ] == 0 ) {
                        // through removing islands, all visible sub-squares on
                        // a visible tile have been turned non-visible
                        // Thus, the tile becomes non-visible too
                        mTileVisibleMap[mapI] =  false;
                        }
                    }
                }
            }
        }
    
    delete [] reachableMap;
    delete [] hitCountMap;


    
    // process vis map to push back shroud at end of narrow corridors
    // so that the corridor end cap isn't too hard to see
    //
    //  We can recognize this case by the following configuration
    //
    //   XXX
    //   VYW
    //   VYW
    //   XXX
    // X and Y are shrouded spots.  If either V or W spots are not
    // shrouded, we should make Y spots unshrouded too.


    int numVisCells = visMapLimit * visMapLimit;
    char *tweakedVisMap = new char[numVisCells];
    
    memcpy( tweakedVisMap, mTargetVisibleMap, numVisCells );

    for( int y=1; y<visMapLimit - VIS_BLOWUP - 1; y++ ) {

        for( int x=1; x<visMapLimit - VIS_BLOWUP - 1; x++ ) {
            int i = y * visMapLimit + x;
            

            if( // top cap of I
                ! mTargetVisibleMap[i-1] && 
                ! mTargetVisibleMap[i] && 
                ! mTargetVisibleMap[i+1] &&
                // bottom cap of I
                ! mTargetVisibleMap[i-1 + visMapLimit*( VIS_BLOWUP + 1 ) ] && 
                ! mTargetVisibleMap[i   + visMapLimit*( VIS_BLOWUP + 1 ) ] && 
                ! mTargetVisibleMap[i+1 + visMapLimit*( VIS_BLOWUP + 1 ) ]  ) {
                
                

                char invisibleVertLine = true;
                char leftOrRightVisible = true;
                for( int yd=1; yd <= VIS_BLOWUP; yd++ ) {    
                    if( mTargetVisibleMap[ i + yd * visMapLimit ] ) {
                        invisibleVertLine = false;
                        break;
                        }
                    if( ! mTargetVisibleMap[ i + yd * visMapLimit - 1 ]
                        &&
                        ! mTargetVisibleMap[ i + yd * visMapLimit + 1 ] ) {
                        leftOrRightVisible = false;
                        break;
                        }
                
                    }

                if( invisibleVertLine && leftOrRightVisible ) {
                    // spine of I invisible, has visible tiles to left
                    // or right
                    // make it visible too
                    for( int yd=1; yd <= VIS_BLOWUP; yd++ ) {    
                        tweakedVisMap[ i + yd * visMapLimit ] = true;
                        }
                    }
                }
            if( // left cap of sideways I
                ! mTargetVisibleMap[i - visMapLimit] && 
                ! mTargetVisibleMap[i] && 
                ! mTargetVisibleMap[i + visMapLimit] &&
                // right cap of sideways I
                ! mTargetVisibleMap[i - visMapLimit + VIS_BLOWUP + 1 ] && 
                ! mTargetVisibleMap[i + VIS_BLOWUP + 1 ] && 
                ! mTargetVisibleMap[i + visMapLimit + VIS_BLOWUP + 1 ] ) {
                
                

                char invisibleHorLine = true;
                char upOrDownVisible = true;
                for( int xd=1; xd <= VIS_BLOWUP; xd++ ) {    
                    if( mTargetVisibleMap[ i + xd ] ) {
                        invisibleHorLine = false;
                        break;
                        }
                    if( ! mTargetVisibleMap[ i + xd - visMapLimit ]
                        &&
                        ! mTargetVisibleMap[ i + xd + visMapLimit ] ) {
                        upOrDownVisible = false;
                        break;
                        }
                
                    }

                if( invisibleHorLine && upOrDownVisible ) {
                    // spine of sideways I invisible, has visible tiles
                    // above or below
                    // make it visible too
                    for( int xd=1; xd <= VIS_BLOWUP; xd++ ) {    
                        tweakedVisMap[ i + xd ] = true;
                        }
                    }
                }
            }
        }
    // wait to apply tweak until AFTER underslip generated
    // so that tweak doesn't shrink underslip)
    
    




    
    int visD = VIS_BLOWUP * HOUSE_D;
    
    memcpy( mTargetVisibleUnderSlipMap, mTargetVisibleMap, numVisCells );

    char *newSlipMap = 
        new char [ numVisCells ];
    
    int shrinkRounds = 3;
    for( int r=0; r<shrinkRounds; r++ ) {
        
        memset( newSlipMap, false, numVisCells );

        int numTrueCells = 0;
        
        for( int i=0; i<numVisCells; i++ ) {
            // expand each visible area to make slip smaller

            if( mTargetVisibleUnderSlipMap[i] ) {
                numTrueCells++;
                
                newSlipMap[i] = true;
                
                int y = i / visD;
                int x = i % visD;
                
                if( y > 0 ) {
                    newSlipMap[i - visD] = true;
                    }
                if( y < visD - 1 ) {
                    newSlipMap[i + visD] = true;
                    }
                if( x > 0 ) {
                    newSlipMap[i - 1] = true;
                    }
                if( x < visD - 1 ) {
                    newSlipMap[i + 1] = true;
                    }
                }
            }
        memcpy( mTargetVisibleUnderSlipMap, newSlipMap, numVisCells );
        }

    delete [] newSlipMap;



    // apply tweaks now, after underslip has been generated from our
    // untweaked shroud

    memcpy( mTargetVisibleMap, tweakedVisMap, numVisCells );
    delete [] tweakedVisMap;
    
    for( int i=0; i<tweakToVisibleLater.size(); i++ ) {
        int index = *( tweakToVisibleLater.getElement( i ) );
        
        mTargetVisibleMap[index] = true;
        }
    


    // pull shroud edge down by one block on all non-walled areas
    // thus, shifting up below will only affect walled areas
    // So, shroud should line up with wall tops where there are walls,
    // but line up with floor tiles elsewhere
    tweakedVisMap = new char[numVisCells];
    
    memcpy( tweakedVisMap, mTargetVisibleMap, numVisCells );

    i = 0;
    
    for( int y=0; y<HOUSE_D * VIS_BLOWUP; y++ ) {

        int flipY = HOUSE_D * VIS_BLOWUP - y - 1;


        int mapY = flipY / VIS_BLOWUP;

        for( int x=0; x<HOUSE_D * VIS_BLOWUP; x++ ) {
            int mapX = x / VIS_BLOWUP;

            int mapI = mapY * HOUSE_D + mapX;
            
            if( !blockingMap[mapI] && !mTargetVisibleMap[i] ) {
                // a black shroud spot
                // over a non-blocking area
           
                if( y > 0 ) {
                    int northN = i - HOUSE_D * VIS_BLOWUP;
                    if( mTargetVisibleMap[northN] ) {
                        // white to north
                        
                        // make this pixel white (move shroud down)
                        tweakedVisMap[i] = true;
                        }
                    }
                if( y < HOUSE_D * VIS_BLOWUP - 1 ) {
                    int southN = i + HOUSE_D * VIS_BLOWUP;
                    if( mTargetVisibleMap[southN] ) {
                        // white to sounth
                        
                        // make south pixel black (move shroud down)
                        tweakedVisMap[southN] = false;
                        }
                    }
                }
            

            i++;
            }
        }
    
    memcpy( mTargetVisibleMap, tweakedVisMap, numVisCells );
    delete [] tweakedVisMap;


    // shift both shrouds up so that it lines up with tops of wall tiles
    for( int y=0; y<visMapLimit-1; y++ ) {
        memcpy( &( mTargetVisibleMap[ y * visMapLimit ] ), 
                &( mTargetVisibleMap[ (y+1) * visMapLimit ] ),
                visMapLimit );
        memcpy( &( mTargetVisibleUnderSlipMap[ y * visMapLimit ] ), 
                &( mTargetVisibleUnderSlipMap[ (y+1) * visMapLimit ] ),
                visMapLimit );
        }

        

    delete [] blockingMap;


    
    // compute which cells want to punch through shroud
    for( int i=0; i< HOUSE_D * HOUSE_D; i++ ) {
        char shouldShowThroughShroud = false;

        if( mTileVisibleMap[i] ) {
            int fullTileI = subToFull( i );

            if( mHouseMapMobileIDs[fullTileI] != 0 ) {
                    
                if( ! isPropertySet( mHouseMapMobileIDs[fullTileI], 
                                     mHouseMapMobileCellStates[fullTileI], 
                                     stuck ) ) {
                    shouldShowThroughShroud = true;
                    }
                }
            
            if( isSubMapPropertySet( i, family ) ) {
                shouldShowThroughShroud = true;
                }    
            }
        
        mTargetShroudPunchThroughMap[i] = shouldShowThroughShroud;
        }
    }



void RobHouseGridDisplay::recomputeVisibilityFloat() {
    
    doublePair robPos = getTilePos( fullToSub( mRobberIndex ) );

    
    doublePair cornerPos = getTilePos( 0 );


    
    int i = 0;
    for( int y=0; y<HOUSE_D * VIS_BLOWUP; y++ ) {
        for( int x=0; x<HOUSE_D * VIS_BLOWUP; x++ ) {
            
            int flipY = HOUSE_D * VIS_BLOWUP - y - 1;

            doublePair visPos = 
                { cornerPos.x +  
                  ( x / (double)VIS_BLOWUP ) 
                  * 2 * mTileRadius - mTileRadius / 2,
                  
                  cornerPos.y +  
                  ( flipY / (double)VIS_BLOWUP ) 
                  * 2 * mTileRadius - mTileRadius / 2 };
            
            int visTileIndex = getTileIndex( visPos.x, visPos.y );
            
            int selfBlockingSteps = 0;

            // steps
            int numSteps = lrint( distance( visPos, robPos ) * 8 );
            char hit = false;
            
            // skip last few steps so that we show borders of visible walls


            doublePair stepVector = mult( sub( visPos, robPos ), 
                                          1.0 / numSteps );
            double stepVectorX = stepVector.x;
            double stepVectorY = stepVector.y;
                            

            // start at step 1, one stepVector length in
            doublePair stepPos = add( robPos, stepVector );

            for( int j=1; j<numSteps - 1 && !hit; j++ ) {
                int stepIndex = getTileIndex( stepPos.x, stepPos.y );
                
                // avoid self-blocking on first steps into self

                if( selfBlockingSteps < 2 && stepIndex == visTileIndex ) {
                    // blocked by self for few steps
                    // don't count this as vision blocking
                    selfBlockingSteps ++;
                    }
                else if( isSubMapPropertySet( stepIndex, visionBlocking ) ) {
                    hit = true;
                    }

                // profiler found:
                // don't call add() in inner loop here
                stepPos.x += stepVectorX;
                stepPos.y += stepVectorY;
                }
            
            if( hit ) {
                mTargetVisibleMap[i] = false;
                }
            else {
                mTargetVisibleMap[i] = true;
                
                // at least one sub-area of tile is visible
                int visTileIndex = getTileIndex( visPos.x, visPos.y );
                
                mTileVisibleMap[ visTileIndex ] = true;
                }

            i++;
            }
        }
    }




void RobHouseGridDisplay::processFamilyAndMobilesAtEnd() {
    if( mFamilyAndMobilesProcessedAtEnd ) {
        // only do this once to avoid family duplication
        return;
        }


    
    if( mHouseMapFinalIDs != NULL ) {
        delete [] mHouseMapFinalIDs;
        mHouseMapFinalIDs = NULL;
        }
    if( mHouseMapFinalCellStates != NULL ) {
        delete [] mHouseMapFinalCellStates;
        mHouseMapFinalCellStates = NULL;
        }
    

    int *newIDs = new int[ mNumMapSpots ];
    int *newCellStates = new int[ mNumMapSpots ];
    
    memcpy( newIDs, mHouseMapIDs, mNumMapSpots * sizeof( int ) );
    memcpy( newCellStates, mHouseMapCellStates, 
            mNumMapSpots * sizeof( int ) );



    int numFamily = mFamilyObjects.size();


    // first, remove any living ones from house to make room
    // for family jump-backs
    for( int i=0; i<numFamily; i++ ) {
        int status = *( mFamilyStatus.getElement( i ) );
        
        int pathLength = *( mFamilyExitPathLengths.getElement( i ) );

        int progress = *( mFamilyExitPathProgress.getElement( i ) );

        GridPos *path = *( mFamilyExitPaths.getElement( i ) );


        if( status != 0 && pathLength > 1 ) {
            // still alive

            // move back to their starting positions

            if( status == 1 ) {
                // alive and in house
                // remove them from where they are in house
                
                int oldIndex = posToIndex( path[progress] );
                newIDs[ oldIndex ] = 0;
                newCellStates[ oldIndex ] = 1;
                }
            // otherwise, left house
            }
        }
    

    // now that they are all removed from house (or dead, and stuck)
    // handle family jump-backs
    for( int i=0; i<numFamily; i++ ) {
        int status = *( mFamilyStatus.getElement( i ) );
        
        int objectID = *( mFamilyObjects.getElement( i ) );
        
        int pathLength = *( mFamilyExitPathLengths.getElement( i ) );

        GridPos *path = *( mFamilyExitPaths.getElement( i ) );


        if( status != 0 && pathLength > 1 ) {
            // still alive
    
            
            // start at their starting positions
            int pathSpot = 0;

            // look for spot that's unblocked for them to stand, as
            // close to starting position along path as possible
            
            char found = false;
            
            // don't consider door mat as possible spot for them
            while( pathSpot < pathLength - 1 ) {
                int newIndex = posToIndex( path[pathSpot] );
                
                if( newIDs[ newIndex ] == 0 &&
                    ( mHouseMapMobileIDs[ newIndex ] == 0
                      ||
                      ! isPropertySet( mHouseMapMobileIDs[ newIndex ],
                                       mHouseMapMobileCellStates[ newIndex ],
                                       noFamilyStartHere ) ) ) {
                    
                    found = true;
                    
                    newIDs[ newIndex ] = objectID;
                    newCellStates[ newIndex ] = 1;
                    break;
                    }
                pathSpot++;
                }
            
            if( !found ) {
                
                // search columns backward away from door
                
                for( int x=1; x<mFullMapD; x++ ) {
                    
                    // above door
                    for( int y=mFullMapD/2; y<mFullMapD; y++ ) {
                        int index = y * mFullMapD + x;
                        
                        if( newIDs[ index ] == 0 &&
                            ( mHouseMapMobileIDs[ index ] == 0
                              ||
                              ! isPropertySet( mHouseMapMobileIDs[ index ],
                                               mHouseMapMobileCellStates[ 
                                                   index ],
                                               noFamilyStartHere ) ) ) {
                            
                            found = true;
                    
                            newIDs[ index ] = objectID;
                            newCellStates[ index ] = 1;
                            break;
                            }
                        }
                    if( found ) {
                        break;
                        }

                    // below door
                    for( int y=mFullMapD/2-1; y>=0; y-- ) {
                        int index = y * mFullMapD + x;
                        
                        if( newIDs[ index ] == 0 &&
                            ( mHouseMapMobileIDs[ index ] == 0
                              ||
                              ! isPropertySet( mHouseMapMobileIDs[ index ],
                                               mHouseMapMobileCellStates[ 
                                                   index ],
                                               noFamilyStartHere ) ) ) {
                            
                            found = true;
                    
                            newIDs[ index ] = objectID;
                            newCellStates[ index ] = 1;
                            break;
                            }
                        }

                    if( found ) {
                        break;
                        }
                    }
                }
            
            // still possible that not found here.
            // in that case (house is so full that there's no room
            // for this family member), well... family member leaves for good!
            }

        }
    

    mHouseMapFinalIDs = newIDs;
    mHouseMapFinalCellStates = newCellStates;


    // now mobiles

    if( mHouseMapMobileFinalIDs != NULL ) {
        delete [] mHouseMapMobileFinalIDs;
        mHouseMapMobileFinalIDs = NULL;
        }
    if( mHouseMapMobileFinalCellStates != NULL ) {
        delete [] mHouseMapMobileFinalCellStates;
        mHouseMapMobileFinalCellStates = NULL;
        }
    

    int *newMobileIDs = new int[ mNumMapSpots ];
    int *newMobileCellStates = new int[ mNumMapSpots ];
    
    memcpy( newMobileIDs, mHouseMapMobileIDs, mNumMapSpots * sizeof( int ) );
    memcpy( newMobileCellStates, mHouseMapMobileCellStates, 
            mNumMapSpots * sizeof( int ) );

    for( int i=0; i<mNumMapSpots; i++ ) {
        if( newMobileIDs[i] != 0 && 
            ! isPropertySet( newMobileIDs[i], 
                             newMobileCellStates[i], stuck ) && 
            ! isPropertySet( newMobileIDs[i], 
                             newMobileCellStates[i], sleeping ) ) {
            
            // clear out any that aren't stuck
            // to make room for mobiles jumping back to land here
            newMobileIDs[i] = 0;
            newMobileCellStates[i] = 0;
            }
        }
    

    for( int i=0; i<mNumMapSpots; i++ ) {
        if( mHouseMapMobileIDs[i] != 0 && 
            ! isPropertySet( mHouseMapMobileIDs[i], 
                             mHouseMapMobileCellStates[i], stuck ) && 
            ! isPropertySet( mHouseMapMobileIDs[i], 
                             mHouseMapMobileCellStates[i], sleeping ) ) {
            
            int jumpBackTarget = mHouseMapMobileStartingPositions[i];
            
            if( newMobileIDs[jumpBackTarget] == 0 ) {
                // clear!
                
                // jump this mobile back to where it started
                newMobileIDs[jumpBackTarget] = mHouseMapMobileIDs[i];
                newMobileCellStates[jumpBackTarget] = 
                    mHouseMapMobileCellStates[i];
                }
            else {
                // starting position not clear 
                // (dead mobile there?)
                // (another mobile that was blocked from returning there?)

                // try staying where we are
                if( newMobileIDs[i] == 0 ) {
                    // current spot empty in new map, stay there
                    newMobileIDs[i] = mHouseMapMobileIDs[i];
                    newMobileCellStates[i] = 
                        mHouseMapMobileCellStates[i];
                    }
                
                // else, where we were trying to jump back to is blocked
                // and where we are now is blocked
                // Nowhere to go, so this mobile disappears (runs away)
                }
            }
        }

    // all jump-backs are done now


    // save these into final state for mobiles (never drawn, but returned
    // by getHouseMap() )

    mHouseMapMobileFinalIDs = newMobileIDs;
    mHouseMapMobileFinalCellStates = newMobileCellStates;

    mFamilyAndMobilesProcessedAtEnd = true;
    }

