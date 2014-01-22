#include "HouseGridDisplay.h"

#include "FastBoxBlurFilter.h"


#include "minorGems/util/stringUtils.h"
#include "minorGems/game/game.h"
#include "minorGems/game/gameGraphics.h"
#include "minorGems/game/drawUtils.h"

#include "minorGems/graphics/openGL/KeyboardHandlerGL.h"

#include "minorGems/io/file/File.h"


#include <math.h>
#include <ctype.h>


#include "houseObjects.h"




#include "minorGems/util/random/CustomRandomSource.h"

extern CustomRandomSource randSource;
extern double frameRateFactor;


char HouseGridDisplay::sInstanceCount = 0;

char HouseGridDisplay::sNoiseTileBankPopulated = false;

SpriteHandle HouseGridDisplay::sNoiseTileBank[ NUM_NOISE_TILES ];
SpriteHandle HouseGridDisplay::sPencilScratchBank[ NUM_NOISE_TILES ];

int *HouseGridDisplay::sHouseMapNoiseTileIndices = NULL;
int HouseGridDisplay::sHouseMapNoiseTileIndicesSize = -1;


SpriteHandle HouseGridDisplay::sDropShadowSprite = NULL;



HouseGridDisplay::HouseGridDisplay( double inX, double inY,
                                    HouseObjectPicker *inPicker )
        : PageComponent( inX, inY ),
          mPicker( inPicker ),
          mHideRobber( false ),
          mWifeMoney( 0 ),
          mWifeName( NULL ),
          mSonName( NULL ),
          mDaughterName( NULL ),
          mHouseMapSet( false ),
          mHouseMapIDs( NULL ),
          mHouseMapCellStates( NULL ),
          mHouseMapMobileIDs( NULL ),
          mHouseMapMobileCellStates( NULL ),
          mHouseMapCellFades( NULL ),
          mHouseMapMobileCellFades( NULL ),
          mBottomRowNonConnectedFaded( false ),
          mHouseMapToolTipOverrideOn( NULL ),
          mHouseMapToolTipOverrideState( NULL ),
          mHouseMapMobileToolTipOverrideOn( NULL ),
          mHouseMapMobileToolTipOverrideState( NULL ),
          mUntouchedHouseMapIDs( NULL ),
          mUntouchedHouseMapCellStates( NULL ),
          mUntouchedHouseMapMobileIDs( NULL ),
          mUntouchedHouseMapMobileCellStates( NULL ),
          mHouseMapSpotsTouched( NULL ),
          mHouseMapNoiseTileIndices( NULL ),
          mSubMapOffsetX( 0 ),
          mSubMapOffsetY( 0 ),
          mHouseSubMapIDs( new int[ HOUSE_D * HOUSE_D ] ),
          mHouseSubMapCellStates( new int[ HOUSE_D * HOUSE_D ] ),
          mHighlightIndex( -1 ),
          mPointerDownIndex( -1 ),
          mDraggedAway( false ),
          mPointerDownObjectID( -1 ),
          mPlaceOnDrag( false ),
          mTileRadius( TILE_RADIUS ),
          mStartIndex( -1 ),
          mGoalIndex( -1 ),
          mMandatoryNeedsPlacing( false ),
          mMandatoryToPlaceID( -1 ),
          mAllFamilyObjectsHaveExitPath( false ),
          mDeadFamilyBlocking( true ),
          mPointerInside( false ),
          mWallShadowSprite( NULL ),
          mSkipShadowComputation( false ),
          mAllowPlacement( true ),
          mLastPlacedObject( 0 ),
          mLastActionChangedDiff( false ),
          mLastActionPlayerMotion( false ),
          mLastResetTargetState( 0 ),
          mToolTargetSprite( loadSprite( "toolTarget.tga" ) ),
          mToolTargetBorderSprite( loadSprite( "toolTargetBorder.tga" ) ),
          mToolTargetPickedFullIndex( -1 ),
          mForbiddenMoveHappened( false ) {


    if( !sNoiseTileBankPopulated ) {
        

        unsigned char noisePixels[ 16 * 16 ];
    
        for( int i=0; i<NUM_NOISE_TILES; i++ ) {
            for( int p=0; p<16*16; p++ ) {
        
                // edges of tiles are transparent
                int x = p % 16;
                int y = p / 16;
            
                if( x == 0 || x == 15 ||
                    y == 0 || y == 15 ) {
                    noisePixels[p] = 0;
                    }
                else {
                    noisePixels[p] = randSource.getRandomBoundedInt( 0, 255 );
                    }
            
                }
        
            sNoiseTileBank[i] = fillSpriteAlphaOnly( noisePixels, 16, 16 );


            // make pencil scratches
            memset( noisePixels, 0, 16 * 16 );
            
            for( int y=7; y<=8; y++ ) {
                for( int x=1; x<15; x++ ) {
                    int p = y * 16 + x;
                    
                    noisePixels[p] = randSource.getRandomBoundedInt( 182, 
                                                                     255 );
                    
                    }
                }
            sPencilScratchBank[i] = fillSpriteAlphaOnly( noisePixels, 16, 16 );
            }


        int dropW = 32;
        int dropH = 32;
        
        unsigned char *dropAlpha = new unsigned char[ dropW * dropH ];
        
        // for blur filter
        int *touchedPixels = new int[ dropW * dropH ];
        int numTouched = 0;

        memset( dropAlpha, 0, dropW * dropH );

        int centerX = dropW / 2;
        int centerY = dropH / 2;
        
        for( int y=0; y<dropH; y++ ) {
            for( int x=0; x<dropW; x++ ) {
                
                int dY = y - centerY;
                int dX = x - centerX;
                
                double distance = sqrt( dY * dY + dX * dX );
                
                int i = y * dropW + x;

                if( distance < 4 ) {
                    dropAlpha[ i ] = 255;
                    }

                if( y > 1 && y < dropH - 1 
                    && 
                    x > 1 && x < dropW - 1 ) {
                    // away from border
                    touchedPixels[ numTouched ] = i;
                    numTouched++;
                    }
                
                }
            }

        FastBoxBlurFilter blur;
        
        
        
        
        for( int i=0; i<20; i++ ) {
            
            blur.applySubRegion( dropAlpha,
                                 touchedPixels,
                                 numTouched,
                                 dropW, dropH );
            }
        
        delete [] touchedPixels;
        

        sDropShadowSprite = fillSpriteAlphaOnly( dropAlpha,
                                                 dropW, 
                                                 dropH );
        delete [] dropAlpha;
        
        sNoiseTileBankPopulated = true;
        }
    
    mAllowMoveKeyHold = true;
    mAllowKeyRepeatAcceleration = false;
    
    mStepsBetweenHeldKeyRepeat = 8;
    
    sInstanceCount++;
    }



HouseGridDisplay::~HouseGridDisplay() {
    if( mWifeName != NULL ) {
        delete [] mWifeName;
        }
    if( mSonName != NULL ) {
        delete [] mSonName;
        }
    if( mDaughterName != NULL ) {
        delete [] mDaughterName;
        }
    if( mHouseMapIDs != NULL ) {
        delete [] mHouseMapIDs;
        }
    if( mHouseMapCellStates != NULL ) {
        delete [] mHouseMapCellStates;
        }

    if( mHouseMapMobileIDs != NULL ) {
        delete [] mHouseMapMobileIDs;
        }
    if( mHouseMapMobileCellStates != NULL ) {
        delete [] mHouseMapMobileCellStates;
        }
    
    if( mHouseMapCellFades != NULL ) {
        delete [] mHouseMapCellFades;
        }
    if( mHouseMapMobileCellFades != NULL ) {
        delete [] mHouseMapMobileCellFades;
        }


    if( mHouseMapToolTipOverrideOn != NULL ) {
        delete [] mHouseMapToolTipOverrideOn;
        }
    if( mHouseMapToolTipOverrideState != NULL ) {
        delete [] mHouseMapToolTipOverrideState;
        }
    
    if( mHouseMapMobileToolTipOverrideOn != NULL ) {
        delete [] mHouseMapMobileToolTipOverrideOn;
        }
    if( mHouseMapMobileToolTipOverrideState != NULL ) {
        delete [] mHouseMapMobileToolTipOverrideState;
        }
    

    if( mUntouchedHouseMapIDs != NULL ) {
        delete [] mUntouchedHouseMapIDs;
        }
    if( mUntouchedHouseMapCellStates != NULL ) {
        delete [] mUntouchedHouseMapCellStates;
        }

    if( mUntouchedHouseMapMobileIDs != NULL ) {
        delete [] mUntouchedHouseMapMobileIDs;
        }
    if( mUntouchedHouseMapMobileCellStates != NULL ) {
        delete [] mUntouchedHouseMapMobileCellStates;
        }

    if( mHouseMapSpotsTouched != NULL ) {
        delete [] mHouseMapSpotsTouched;
        }




    if( mHouseMapNoiseTileIndices != NULL ) {
        delete [] mHouseMapNoiseTileIndices;
        }
    

    delete [] mHouseSubMapIDs;
    delete [] mHouseSubMapCellStates;

    if( mWallShadowSprite != NULL ) {
        freeSprite( mWallShadowSprite );
        mWallShadowSprite = NULL;
        }
    
    sInstanceCount--;
    
    if( sInstanceCount == 0 && sNoiseTileBankPopulated ) {
        
        for( int i=0; i<NUM_NOISE_TILES; i++ ) {
            freeSprite( sNoiseTileBank[i] );
            freeSprite( sPencilScratchBank[i] );
            }
        sNoiseTileBankPopulated = false;

        if( sHouseMapNoiseTileIndices != NULL ) {
            delete [] sHouseMapNoiseTileIndices;
            sHouseMapNoiseTileIndices = NULL;
            sHouseMapNoiseTileIndicesSize = -1;
            }

        freeSprite( sDropShadowSprite );
        }

    freeSprite( mToolTargetSprite );
    freeSprite( mToolTargetBorderSprite );
    
    for( int i=0; i<mFamilyExitPaths.size(); i++ ) {
        delete [] *( mFamilyExitPaths.getElement( i ) );
        }
    mFamilyExitPaths.deleteAll();
    mFamilyExitPathLengths.deleteAll();
    }



void HouseGridDisplay::setWifeName( const char *inWifeName ) {
    if( mWifeName != NULL ) {
        delete [] mWifeName;
        }
    mWifeName = stringDuplicate( inWifeName );
    }


void HouseGridDisplay::setSonName( const char *inSonName ) {
    if( mSonName != NULL ) {
        delete [] mSonName;
        }
    mSonName = stringDuplicate( inSonName );
    }


void HouseGridDisplay::setDaughterName( const char *inDaughterName ) {
    if( mDaughterName != NULL ) {
        delete [] mDaughterName;
        }
    mDaughterName = stringDuplicate( inDaughterName );
    }

    



void HouseGridDisplay::setHouseMap( const char *inHouseMap ) {
    mHouseMapSet = true;
    
    int numTokens;
    char **tokens = split( inHouseMap, "#", &numTokens );
    
    if( mHouseMapIDs != NULL ) {
        delete [] mHouseMapIDs;
        }

    if( mHouseMapCellStates != NULL ) {
        delete [] mHouseMapCellStates;
        }

    if( mHouseMapMobileIDs != NULL ) {
        delete [] mHouseMapMobileIDs;
        }

    if( mHouseMapMobileCellStates != NULL ) {
        delete [] mHouseMapMobileCellStates;
        }

    if( mHouseMapCellFades != NULL ) {        
        delete [] mHouseMapCellFades;
        }
    if( mHouseMapMobileCellFades != NULL ) {        
        delete [] mHouseMapMobileCellFades;
        }

    if( mHouseMapToolTipOverrideOn != NULL ) {
        delete [] mHouseMapToolTipOverrideOn;
        }
    if( mHouseMapToolTipOverrideState != NULL ) {
        delete [] mHouseMapToolTipOverrideState;
        }
    
    if( mHouseMapMobileToolTipOverrideOn != NULL ) {
        delete [] mHouseMapMobileToolTipOverrideOn;
        }
    if( mHouseMapMobileToolTipOverrideState != NULL ) {
        delete [] mHouseMapMobileToolTipOverrideState;
        }


    if( mUntouchedHouseMapIDs != NULL ) {
        delete [] mUntouchedHouseMapIDs;
        }

    if( mUntouchedHouseMapCellStates != NULL ) {
        delete [] mUntouchedHouseMapCellStates;
        }

    if( mUntouchedHouseMapMobileIDs != NULL ) {
        delete [] mUntouchedHouseMapMobileIDs;
        }

    if( mUntouchedHouseMapMobileCellStates != NULL ) {
        delete [] mUntouchedHouseMapMobileCellStates;
        }


    if( mHouseMapSpotsTouched != NULL ) {
        delete [] mHouseMapSpotsTouched;
        }


    if( mHouseMapNoiseTileIndices != NULL ) {
        delete [] mHouseMapNoiseTileIndices;
        }
    
    mNumMapSpots = numTokens;
    mFullMapD = (int)sqrt( mNumMapSpots );

    if( mFullMapD < HOUSE_D ) {
        // house map not even big enoug to fill screen
        // probably corrupted!
    
        printf( "House map corrupted\n" );

        for( int i=0; i<mNumMapSpots; i++ ) {
            delete [] tokens[i];
            }
        delete [] tokens;
        
        mNumMapSpots = HOUSE_D * HOUSE_D;
        mFullMapD = HOUSE_D;

        tokens = new char*[mNumMapSpots];
        
        int startLoc = mFullMapD * ( mFullMapD / 2 );
        int vaultLoc = startLoc + 1;
        
        for( int i=0; i<mNumMapSpots; i++ ) {
            if( i == startLoc ) {
                tokens[i] = stringDuplicate( "0" );
                }
            else if( i == vaultLoc ) {
                tokens[i] = autoSprintf( "%d", GOAL_ID );
                }
            else {
                // fill with solid concrete to represent this
                tokens[i] = stringDuplicate( "998" );
                }
            }
        }
    
    
    mHouseMapIDs = new int[ mNumMapSpots ];
    mHouseMapCellStates = new int[ mNumMapSpots ];
    mHouseMapMobileIDs = new int[ mNumMapSpots ];
    mHouseMapMobileCellStates = new int[ mNumMapSpots ];
    mHouseMapCellFades = new float[ mNumMapSpots ];
    mHouseMapMobileCellFades = new float[ mNumMapSpots ];
    mHouseMapNoiseTileIndices = new int[ mNumMapSpots ];
    
    mHouseMapToolTipOverrideOn = new char[ mNumMapSpots ];
    mHouseMapToolTipOverrideState = new int[ mNumMapSpots ];
    mHouseMapMobileToolTipOverrideOn = new char[ mNumMapSpots ];
    mHouseMapMobileToolTipOverrideState = new int[ mNumMapSpots ];
    

    for( int i=0; i<mNumMapSpots; i++ ) {
        
        mHouseMapIDs[i] = 0;
        mHouseMapMobileIDs[i] = 0;
        mHouseMapCellStates[i] = 0;
        mHouseMapMobileCellStates[i] = 0;
        
        mHouseMapCellFades[i] = 1.0f;
        mHouseMapMobileCellFades[i] = 1.0f;

        mHouseMapToolTipOverrideOn[i] = false;
        mHouseMapMobileToolTipOverrideOn[i] = false;
        

        int numObjects;
        char **objectStrings = split( tokens[i], ",", &numObjects );

        // ignore all but two objects
        // second object is mobile object, if present

        for( int j=0; j<numObjects; j++ ) {
            
            int id, state;
                    
            int numRead = sscanf( objectStrings[j], "%d:%d", 
                                  &id, 
                                  &state );
        
            if( numRead < 2 ) {
                // no ":value" present after ID
                // use default
                state = 0;
        
                if( numRead == 0 ) {
                    // reading ID failed?
                    
                    // default
                    id = 0;
                    }
                }
        
            if( id == GOAL_ID ) {
                mGoalIndex = i;
                }

            if( isPropertySet( id, state, mobile ) ) {
                mHouseMapMobileIDs[i] = id;
                
                
                if( isPropertySet( id, state, interactingWithPlayer ) &&
                    ! isPropertySet( id, state, stuck ) ) {
                    // left over state where this mobile interacted with
                    // last robber

                    // but that robber no longer present, so discard
                    // this state
                    state = 0;
                    }

                mHouseMapMobileCellStates[i] = state;
                }
            else {
                // non-mobile
                mHouseMapIDs[i] = id;
                mHouseMapCellStates[i] = state;
                }
            delete [] objectStrings[j];
            }
        delete [] objectStrings;
        

        delete [] tokens[i];
        
        if( sHouseMapNoiseTileIndices == NULL ||
            sHouseMapNoiseTileIndicesSize != mNumMapSpots ) {
            
            // cache of indices doesn't exist or is wrong size
            // generate new

            mHouseMapNoiseTileIndices[i] = 
                randSource.getRandomBoundedInt( 0, NUM_NOISE_TILES - 1 );
            }
        else {
            // cache fits, use it
            mHouseMapNoiseTileIndices[i] = sHouseMapNoiseTileIndices[i];
            }
        }
    
    delete [] tokens;


    if( sHouseMapNoiseTileIndices == NULL ||
        sHouseMapNoiseTileIndicesSize != mNumMapSpots ) {

        // create or replace cache with the index values we just
        // generated.

        if( sHouseMapNoiseTileIndices != NULL ) {
            delete [] sHouseMapNoiseTileIndices;
            }

        sHouseMapNoiseTileIndices = new int[ mNumMapSpots ];
        sHouseMapNoiseTileIndicesSize = mNumMapSpots;
        
        memcpy( sHouseMapNoiseTileIndices, mHouseMapNoiseTileIndices,
                mNumMapSpots * sizeof( int ) );
        }
    

    mUntouchedHouseMapIDs = new int[ mNumMapSpots ];
    mUntouchedHouseMapCellStates = new int[ mNumMapSpots ];
    mUntouchedHouseMapMobileIDs = new int[ mNumMapSpots ];
    mUntouchedHouseMapMobileCellStates = new int[ mNumMapSpots ];
    
    memcpy( mUntouchedHouseMapIDs, mHouseMapIDs, 
            mNumMapSpots * sizeof( int ) );

    memcpy( mUntouchedHouseMapCellStates, mHouseMapCellStates, 
            mNumMapSpots * sizeof( int ) );

    memcpy( mUntouchedHouseMapMobileIDs, mHouseMapMobileIDs, 
            mNumMapSpots * sizeof( int ) );

    memcpy( mUntouchedHouseMapMobileCellStates, mHouseMapMobileCellStates, 
            mNumMapSpots * sizeof( int ) );


    mHouseMapSpotsTouched = new char[ mNumMapSpots ];
    
    memset( mHouseMapSpotsTouched, false, mNumMapSpots );
    mTouchedHighlightFade = 0;
    mTouchedHighlightRed = false;
    mTouchedHighlightOn = false;
    
    // center vertically, far left
    setVisibleOffset( 0, ( mFullMapD - HOUSE_D ) / 2 );

    mStartIndex = mFullMapD * ( mFullMapD / 2 );
    
    mRobberIndex = mStartIndex;
    mLastRobberIndex = mStartIndex;
    mRobberState = 0;
    mRobberOrientation = 1;
    
    mEditHistory.deleteAll();

    checkExitPaths();

    
    mMapStartedOutEmpty = true;
    
    for( int i=0; i<mNumMapSpots; i++ ) {
        
        if( mHouseMapMobileIDs[i] != 0 ) {
            mMapStartedOutEmpty = false;
            break;
            }
        
        if( mHouseMapIDs[i] != 0
            &&
            ! isPropertySet( mHouseMapIDs[i], 
                             mHouseMapCellStates[i], mandatory )
            &&
            ! isPropertySet( mHouseMapIDs[i], 
                             mHouseMapCellStates[i], permanent ) ) {
            
            mMapStartedOutEmpty = false;
            break;
            }
        }
    
    mMandatoryNeedsPlacing = false;

    mForbiddenMoveHappened = false;

    for( int i=0; i<MG_KEY_LAST_CODE + 1; i++ ) {
        mSpecialKeysHeldSteps[i] = 0;
        mSpecialKeysHeldStepsTotal[i] = 0;
        }
    }



void HouseGridDisplay::setWifeMoney( int inMoney ) {
    mWifeMoney = inMoney;
    }



void HouseGridDisplay::resetToggledStatesInternal( int inTargetState,
                                                   char inForceUnstuck ) {
    for( int i=0; i<mNumMapSpots; i++ ) {
        int id = mHouseMapIDs[i];
        int state = mHouseMapCellStates[i];
        
        char isStuck = isPropertySet( id, state, stuck );
        char isNoAutoRevert = isPropertySet( id, state, noAutoRevert );

        if( inForceUnstuck || ! ( isStuck || isNoAutoRevert ) ) {
            
            if( isStuck && ! isPropertySet( id, state, deadFamily ) ) {
                mHouseMapToolTipOverrideOn[i] = true;
                mHouseMapToolTipOverrideState[i] = mHouseMapCellStates[i];
                }
            
            mHouseMapCellStates[i] = inTargetState;
            }

        // same for mobile objects
        isStuck = isPropertySet( mHouseMapMobileIDs[i], 
                                 mHouseMapMobileCellStates[i], stuck );
        
        isNoAutoRevert = isPropertySet( mHouseMapMobileIDs[i], 
                                        mHouseMapMobileCellStates[i], 
                                        noAutoRevert );
        
        if( mHouseMapMobileIDs[i] != 0 
            &&
            ( inForceUnstuck || ! ( isStuck || isNoAutoRevert ) ) ) {
            
            if( isStuck ) {
                mHouseMapMobileToolTipOverrideOn[i] = true;
                mHouseMapMobileToolTipOverrideState[i] = 
                    mHouseMapMobileCellStates[i];
                }

            mHouseMapMobileCellStates[i] = inTargetState;
            }
        
        // leave other states alone
        // example:  walls that were burned down during a previous, successful
        // robbery, but not repaired by owner yet
        }
    mLastResetTargetState = inTargetState;
    }



void HouseGridDisplay::resetToggledStates( int inTargetState,
                                           char inForceUnstuck ) {
    resetToggledStatesInternal( inTargetState, inForceUnstuck );
    
    copyAllIntoSubCells();

    recomputeWallShadows();
    }



char *HouseGridDisplay::getWifeName() {
    if( mWifeName == NULL ) {
        return NULL;
        }
    else {
        return stringDuplicate( mWifeName );
        }
    }

char *HouseGridDisplay::getSonName() {
    if( mSonName == NULL ) {
        return NULL;
        }
    else {
        return stringDuplicate( mSonName );
        }
    }

char *HouseGridDisplay::getDaughterName() {
    if( mDaughterName == NULL ) {
        return NULL;
        }
    else {
        return stringDuplicate( mDaughterName );
        }
    }



char *HouseGridDisplay::getHouseMap() {
    if( !mHouseMapSet ) {
        return NULL;
        }
    else {
        


        char **parts = new char*[ mNumMapSpots ];
        

        for( int i=0; i<mNumMapSpots; i++ ) {
            char *mobilePart = NULL;
            char *nonMobilePart = NULL;
            
            
            if( mHouseMapCellStates[i] != 0 && mHouseMapCellStates[i] != 1 ) {
                const char *stuckFlag = "";
                
                if( isPropertySet( mHouseMapIDs[i], mHouseMapCellStates[i],
                                   stuck ) ) {
                    stuckFlag = "!";
                    }

                // not default state, include state
                nonMobilePart = autoSprintf( "%d:%d%s", 
                                             mHouseMapIDs[i],
                                             mHouseMapCellStates[i],
                                             stuckFlag );
                }
            else {
                // one of two default states, skip including it
                nonMobilePart = autoSprintf( "%d", mHouseMapIDs[i] );
                }

            if( mHouseMapMobileIDs[i] != 0 ) {
                // mobile object present
                
                if( mHouseMapMobileCellStates[i] != 0 && 
                    mHouseMapMobileCellStates[i] != 1 ) {
                    
                    const char *stuckFlag = "";
                    
                    if( isPropertySet( mHouseMapMobileIDs[i], 
                                       mHouseMapMobileCellStates[i],
                                       stuck ) ) {
                        stuckFlag = "!";
                        }


                    // not default state, include state
                    mobilePart = 
                        autoSprintf( "%d:%d%s", 
                                     mHouseMapMobileIDs[i],
                                     mHouseMapMobileCellStates[i],
                                     stuckFlag );
                    }
                else {
                    // one of two default states, skip including it
                    mobilePart = autoSprintf( "%d", mHouseMapMobileIDs[i] );
                    }

                }

            if( mobilePart == NULL ) {
                parts[i] = nonMobilePart;
                }
            else {
                if( mHouseMapIDs[i] == 0 && 
                    ( mHouseMapCellStates[i] == 0 || 
                      mHouseMapCellStates[i] == 1 )  ) {
                    // only mobile part present, above empty floor

                    parts[i] = mobilePart;
                    if( nonMobilePart != NULL ) {
                        delete [] nonMobilePart;
                        }
                    }
                else {
                    // both mobile and non-mobile

                    // comma-separated parts
                    parts[i] = autoSprintf( "%s,%s", 
                                            nonMobilePart, mobilePart );
                    delete [] nonMobilePart;
                    delete [] mobilePart;
                    }
                }
            }
        
        // don't use join here because it leaves result lingering
        // in freed memory 
        //char *houseMap = join( parts, mNumMapSpots, "#" );

        int resultLength = 0;
        for( int i=0; i<mNumMapSpots; i++ ) {
            resultLength += strlen( parts[i] );
            }
        // also space for # separators
        resultLength += ( mNumMapSpots - 1 );
        
        char *houseMap = new char[ resultLength + 1];
        int pos = 0;
        for( int i=0; i<mNumMapSpots; i++ ) {
            if( i != 0 ) {
                houseMap[pos] = '#';
                pos++;
                }
            int partLength = strlen( parts[i] );
            memcpy( &( houseMap[pos] ), parts[i], partLength );
            
            delete [] parts[i];
            
            pos += partLength;
            }
        houseMap[resultLength] = '\0';
        delete [] parts;
        

        return houseMap;
        }
    }



char *HouseGridDisplay::getEditList() {
    int numEdits = mEditHistory.size();

    if( numEdits == 0 ) {
        return stringDuplicate( "" );
        }
    
    
    SimpleVector<char*> listAccum;
    
    for( int i=0; i<numEdits; i++ ) {
        GridChangeRecord *r = mEditHistory.getElement( i );
        
        char *editString = autoSprintf( "%d@%d", r->newID, r->fullIndex );
    
        listAccum.push_back( editString );
        }
    
    char **listStrings = listAccum.getElementArray();
    
    char *editList = join( listStrings, numEdits, "#" );

    for( int i=0; i<numEdits; i++ ) {
        delete [] listStrings[i];
        }
    delete [] listStrings;

    return editList;
    }




static void addToDiff( SimpleVector<GridDiffRecord> *inDiffList,
                       int inObjectID ) {
    
    int numInList = inDiffList->size();
    
    for( int i=0; i<numInList; i++ ) {
        GridDiffRecord *r = inDiffList->getElement( i );
        
        if( r->objectID == inObjectID ) {
            r->placementCount ++;
            return;
            }
        }

    // not found
    GridDiffRecord r = { inObjectID, 1 };
    inDiffList->push_back( r );
    }



SimpleVector<GridDiffRecord> HouseGridDisplay::getEditDiff() {

    // recompute touched spots
    memset( mHouseMapSpotsTouched, false, mNumMapSpots );


    SimpleVector<GridDiffRecord> diffList;

    SimpleVector<GridDiffRecord> oldMobCount;
    SimpleVector<GridDiffRecord> newMobCount;
    
    for( int i=0; i<mNumMapSpots; i++ ) {
        
        int oldID = mUntouchedHouseMapIDs[i];
        int oldState = mUntouchedHouseMapCellStates[i];
        
        char oldIsStuck = isPropertySet( oldID, 
                                         oldState, 
                                         stuck );

        int newID = mHouseMapIDs[i];
        int newState = mHouseMapCellStates[i];
        
        char newIsStuck = isPropertySet( newID, 
                                         newState, 
                                         stuck );
        char newIsMobile = isPropertySet( newID, 
                                          newState, 
                                          mobile );
        char newIsMandatory = isPropertySet( newID, 
                                             newState, 
                                             mandatory );


        int oldMobID = mUntouchedHouseMapMobileIDs[i];
        int oldMobState = mUntouchedHouseMapMobileCellStates[i];
        
        char oldMobIsStuck = isPropertySet( oldMobID, 
                                            oldMobState, 
                                            stuck );

        int newMobID = mHouseMapMobileIDs[i];
        int newMobState = mHouseMapMobileCellStates[i];
        
        char newMobIsStuck = isPropertySet( newMobID, 
                                            newMobState, 
                                            stuck );


        // non-free change in this cell?
        // (ignore mobiles, count them separately below)
        if( ! newIsMobile &&
            ! newIsMandatory &&
            ( oldID != newID 
              ||
              ( oldIsStuck && oldState != newState ) ) ) {
            
            addToDiff( &diffList, newID );

            mHouseMapSpotsTouched[i] = true;
            }

        // only ONE mobile here in old and new states
        // either in base layer or mobile layer, but not both
        int mobileIDThatWasHere = 0;
        int mobileIDThatIsHereNow = 0;
                
        // count base-layer mobiles in this cell
        if( isPropertySet( oldID, oldState, mobile ) &&
            ! oldIsStuck ) {
            
            addToDiff( &oldMobCount, oldID );

            mobileIDThatWasHere = oldID;
            }

        if( isPropertySet( newID, newState, mobile ) &&
            ! newIsStuck ) {
            
            addToDiff( &newMobCount, newID );
            
            mobileIDThatIsHereNow = newID;
            }

        
        // count mobile-layer mobiles in this cell
        if( oldMobID != 0 && 
            ! oldMobIsStuck ) {
            
            addToDiff( &oldMobCount, oldMobID );

            mobileIDThatWasHere = oldMobID;
            }
        if( newMobID != 0 && 
            ! newMobIsStuck ) {
            
            addToDiff( &newMobCount, newMobID );

            mobileIDThatIsHereNow = newMobID;
            }

        
        if( mobileIDThatIsHereNow != 0 &&
            mobileIDThatIsHereNow != mobileIDThatWasHere ) {
            
            // mobile placement (or move?)
            // don't include in diff, but mark as touched
            mHouseMapSpotsTouched[i] = true;
            }

        }

    // now look for excess mobiles in new state, and add them to diff

    int numNewRecords = newMobCount.size();
    int numOldRecords = oldMobCount.size();

    for( int i=0; i<numNewRecords; i++ ) {
        GridDiffRecord rNew = *( newMobCount.getElement( i ) );
    
        int rNewID = rNew.objectID;
        
        // search in old records for match
        for( int j=0; j<numOldRecords; j++ ) {
            GridDiffRecord rOld = *( oldMobCount.getElement( j ) );
        
            if( rOld.objectID == rNewID ) {
                
                rNew.placementCount -= rOld.placementCount;
                break;
                }
            }
        
        if( rNew.placementCount > 0 ) {
            // excess

            diffList.push_back( rNew );
            }
        
        // only show rNew.placementCount touched spots in grid as
        // touched (other touched spots beyond that are just moved, which
        // we don't want to highlight, because hightlight should represent
        // only what user is paying for).
        // if rNew.placementCount is NEGATIVE or ZERO, 
        // should show NONE of them as touched
        int touchedMatchesFound = 0;
        for( int i=0; i<mNumMapSpots; i++ ) {
            if( mHouseMapSpotsTouched[i] && 
                mHouseMapIDs[ i ] == rNew.objectID ) {
                
                if( touchedMatchesFound < rNew.placementCount ) {
                    // allow to remain touched
                    }
                else {
                    // untouch all additional ones encountered
                    mHouseMapSpotsTouched[i] = false;
                    }
                touchedMatchesFound++;
                }
            }
            
        }
    
    if( mTouchedHighlightOn ) {    
        mTouchedHighlightFade = 1.0;
        }
    else {
        mTouchedHighlightFade = 0;
        }
    
    return diffList;
    }




void HouseGridDisplay::toggleTouchedHighlights( char inOn ) {
    char wasOn = mTouchedHighlightOn;
    
    mTouchedHighlightOn = inOn;

    if( mTouchedHighlightOn && !wasOn ) {
        mTouchedHighlightFade = 1.0;
        }
    else if( ! mTouchedHighlightOn && wasOn ) {
        mTouchedHighlightFade = 0;
        }
    }




char *HouseGridDisplay::getFamilyExitPaths() {
    int numPaths = mFamilyExitPaths.size();
    
    if( numPaths == 0 ) {
        return stringDuplicate( "##" );
        }
    
    SimpleVector<char*> mainListAccum;

    for( int i=0; i<numPaths; i++ ) {
        GridPos *moveList = *( mFamilyExitPaths.getElement( i ) );
        int length = *( mFamilyExitPathLengths.getElement( i ) );
        
        if( length > 1 ) {
            SimpleVector<char *> subListAccum;
        
            for( int m=0; m<length; m++ ) {
                char *posString = 
                    autoSprintf( "%d", posToIndex( moveList[m] ) );
                
                subListAccum.push_back( posString );
                }

            char **subListStrings = subListAccum.getElementArray();
            
            char *posList = join( subListStrings, length, "#" );
            
            for( int i=0; i<length; i++ ) {
                delete [] subListStrings[i];
                }
            delete [] subListStrings;
            
            mainListAccum.push_back( posList );
            }
        
        }

    if( mainListAccum.size() == 0 ) {
        return stringDuplicate( "##" );
        }


    // length-1 paths are skipped above, so numPaths might decrease
    numPaths = mainListAccum.size();
    
    char **listStrings = mainListAccum.getElementArray();
    
    char *pathsList = join( listStrings, numPaths, "##" );

    for( int i=0; i<numPaths; i++ ) {
        delete [] listStrings[i];
        }
    delete [] listStrings;

    return pathsList;
    }



char HouseGridDisplay::getWifeLiving() {
    for( int i=0; i<mNumMapSpots; i++ ) {
        int id = mHouseMapIDs[i];
        int state = mHouseMapCellStates[i];
        
        if( isPropertySet( id, state, wife ) ) {
            
            if( isPropertySet( id, state, deadFamily ) ) {    
                return false;
                }
            else if( isPropertySet( id, state, family ) ) {
                return true;
                }
            }
        }

    return false;
    }



char HouseGridDisplay::areMandatoriesPlaced() {
    return !mMandatoryNeedsPlacing;
    }


char HouseGridDisplay::doAllFamilyObjectsHaveExitPath() {
    return mAllFamilyObjectsHaveExitPath;
    }



char HouseGridDisplay::didForbiddenMoveHappen() {
    return mForbiddenMoveHappened;
    }



void HouseGridDisplay::allowPlacement( char inAllow ) {
    mAllowPlacement = inAllow;
    }



int HouseGridDisplay::getLastPlacedObject() {
    return mLastPlacedObject;
    }

    

void HouseGridDisplay::step() {

    for( int i=0; i<MG_KEY_LAST_CODE+1; i++ ) {
        if( mSpecialKeysHeldSteps[i] > 0 ) {
            mSpecialKeysHeldSteps[i] ++;
            mSpecialKeysHeldStepsTotal[i] ++;
            }
        }


    

    int threshold = lrint( 30 / frameRateFactor );

    int betweenJumpSteps = 
        lrint( mStepsBetweenHeldKeyRepeat / frameRateFactor );
    
    if( mAllowMoveKeyHold ) {
        for( int i=MG_KEY_LEFT; i<=MG_KEY_DOWN; i++ ) {
        
            if( mSpecialKeysHeldSteps[i] > threshold ) {
                
                int thisNextJumpSteps = betweenJumpSteps;
                
                if( mAllowKeyRepeatAcceleration &&
                    mSpecialKeysHeldStepsTotal[i] > threshold * 2 ) {
                    // held down for long enough
                    // delays between repeats cut in half from now on
                    thisNextJumpSteps /= 2;
                    }
                
                // repeat that key
                specialKeyDown( i );
                
                // wait some steps before next repeat
                mSpecialKeysHeldSteps[i] -= thisNextJumpSteps;
                }
            
            
            }
        
        

        }

    mTouchedHighlightFade -= 0.01 / frameRateFactor;
    
    if( mTouchedHighlightFade < 0 ) {
        mTouchedHighlightFade = 0;
        }    
    }



// profiler found:
// pull these computations outside of the call
static double indexOffset = HOUSE_D * TILE_RADIUS - TILE_RADIUS;
static double indexScale = 1.0 / ( 2 * TILE_RADIUS );


int HouseGridDisplay::getTileIndex( double inX, double inY ) {

    int x = lrint( ( inX + indexOffset ) * indexScale );
    
    int y = lrint( ( inY + indexOffset ) * indexScale );
    
    if( x >= 0 && x < HOUSE_D 
        &&
        y >= 0 && y < HOUSE_D ) {
        
        return y * HOUSE_D + x;
        }
    else {
        return -1;
        }    
    }



doublePair HouseGridDisplay::getTilePos( int inIndex ) {
    
    int x = inIndex % HOUSE_D;
    int y = inIndex / HOUSE_D;
    


    doublePair tilePos = {  x * 2 * mTileRadius - 
                              ( HOUSE_D * mTileRadius - mTileRadius ), 
                            y * 2 * mTileRadius - 
                              ( HOUSE_D * mTileRadius - mTileRadius ) };

    return tilePos;
    }



doublePair HouseGridDisplay::getTilePosFull( int inFullIndex ) {

    int fullX = inFullIndex % mFullMapD;
    int fullY = inFullIndex / mFullMapD;
    
    

    int xOffsetFromZero = fullX - mSubMapOffsetX;
    int yOffsetFromZero = fullY - mSubMapOffsetY;
    
    
    doublePair tilePos = getTilePos( 0 );
    
    tilePos.x += xOffsetFromZero * 2 * mTileRadius;
    tilePos.y += yOffsetFromZero * 2 * mTileRadius;
    

    return tilePos;
    }



int HouseGridDisplay::getTileNeighbor( int inFullIndex, int inNeighbor ) {
    
    int fullY = inFullIndex / mFullMapD;
    int fullX = inFullIndex % mFullMapD;
    
    int dX[4] = { 0, 1, 0, -1 };
    int dY[4] = { 1, 0, -1, 0 };

    int nY = fullY + dY[ inNeighbor ];

    int nX = fullX + dX[ inNeighbor ];
    
    
    if( nY < 0 || nY >= mFullMapD
        ||
        nX < 0 || nX >= mFullMapD ) {
        
        // out of bounds
        return -1;
        }

    return nY * mFullMapD + nX;
    }



int HouseGridDisplay::getTileNeighborEight( int inFullIndex, int inNeighbor ) {
    
    int fullY = inFullIndex / mFullMapD;
    int fullX = inFullIndex % mFullMapD;
    
    int dX[8] = { 0, 1, 1,  1,  0, -1, -1, -1 };
    int dY[8] = { 1, 1, 0, -1, -1, -1,  0,  1 };

    int nY = fullY + dY[ inNeighbor ];

    int nX = fullX + dX[ inNeighbor ];
    
    
    if( nY < 0 || nY >= mFullMapD
        ||
        nX < 0 || nX >= mFullMapD ) {
        
        // out of bounds
        return -1;
        }

    return nY * mFullMapD + nX;
    }




int HouseGridDisplay::getTileNeighborHasProperty( int inFullIndex, 
                                                  int inNeighbor,
                                                  propertyID inProperty ) {
    int fullY = inFullIndex / mFullMapD;
    int fullX = inFullIndex % mFullMapD;
    
    int dX[4] = { 0, 1, 0, -1 };
    int dY[4] = { 1, 0, -1, 0 };

    int nY = fullY + dY[ inNeighbor ];

    int nX = fullX + dX[ inNeighbor ];
    
    
    if( nY < 0 || nY >= mFullMapD
        ||
        nX < 0 || nX >= mFullMapD ) {
        
        // out of bounds, empty floor
        return false;
        }

    int nIndex = nY * mFullMapD + nX;

    return isPropertySet( mHouseMapIDs[ nIndex ],
                          mHouseMapCellStates[ nIndex ], 
                          inProperty );
    }





int HouseGridDisplay::computeBlockedOrientation( int inFullIndex,
                                                 propertyID inBlockingProperty,
                                                 char *outCompletelyBlocked ) {
    
    int numBlockedNeighbors = 0;

    char neighborsBlocked[4] = { false, false, false, false };
                
    int oneBlockedIndex = 0;

    for( int n=0; n<4; n++ ) {
        if( getTileNeighborHasProperty( inFullIndex, n, 
                                        inBlockingProperty ) ) {
            numBlockedNeighbors ++;
                        
            neighborsBlocked[n] = true;
                        
            oneBlockedIndex = n;
            }
        }
                
    *outCompletelyBlocked = ( numBlockedNeighbors == 4 );


    if( numBlockedNeighbors == 0 || numBlockedNeighbors == 4 ) {
        // default to left-facing when none/all are blocked
        return 3;
        }
    else if( numBlockedNeighbors == 1 ) {
        // face away from only blockage
        switch( oneBlockedIndex ) {
            case 0:
                return 2;
                break;
            case 1:
                return 3;
                break;
            case 2:
                return 0;
                break;
            case 3:
                return 1;
                break;
            }                            
        }
    else {
        // blocked on multiple sides
        // face whatever direction is open
                    
        for( int n=0; n<4; n++ ) {
            if( !neighborsBlocked[n] ) {
                return n;
                }
            }
        }

    // we'll never get here because of cases above, but satisfy the compiler
    return 3;
    }

    




int HouseGridDisplay::getOrientationIndex( int inFullIndex, 
                                           int inTileID, int inTileState ) {
    int numOrientations = 0;
    
    int orientationIndex = 0;
    
    numOrientations = getNumOrientations( inTileID, inTileState );            
            
                
    if( numOrientations == 16 ) {
        // full binary LBRT flags based on neighbors of same type
        // OR in groupWith set for our tile
        // (different states okay, but only if tile in that state has
        //  the same number of orientations)

        int neighborsEqual[4] = { 0, 0, 0, 0 };

        for( int n=0; n<4; n++ ) {
            
            int neighborIndex = getTileNeighbor( inFullIndex, n );
            
            if( neighborIndex != -1 &&
                isInGroup( inTileID, mHouseMapIDs[neighborIndex] ) && 
                getNumOrientations( 
                    mHouseMapIDs[neighborIndex], 
                    mHouseMapCellStates[neighborIndex] ) == numOrientations ) {

                neighborsEqual[n] = 1;
                }
            }                
                
        orientationIndex = 
            neighborsEqual[3] << 3 |
            neighborsEqual[2] << 2 |
            neighborsEqual[1] << 1 |
            neighborsEqual[0];
        }
    else if( numOrientations == 4 ) {
                



        if( isPropertySet( inTileID, inTileState, playerFacing ) ||
            isPropertySet( inTileID, inTileState, playerFacingAway ) ) {
            
            int x = inFullIndex % mFullMapD;
            int y = inFullIndex / mFullMapD;
            
            int robberX = mRobberIndex % mFullMapD;
            int robberY = mRobberIndex / mFullMapD;
            
            int dX = robberX - x;
            int dY = robberY - y;
            
            if( isPropertySet( inTileID, inTileState, playerFacingAway ) ) {
                // face away from player
                dX *= -1;
                dY *= -1;
                }

            if( dX == 0 && dY == 0 ) {
                // same square as player
                // face same direction as player
                orientationIndex = mRobberOrientation;
                // but only if player facing left/right
                // (otherwise, always face right)
                if( orientationIndex != 1 && orientationIndex != 3 ) {
                    orientationIndex = 1;
                    }
                }
            else {
                if( abs( dX ) > abs( dY ) ) {
                    // x facing
                
                    if( dX < 0 ) {
                        orientationIndex = 3;
                        }
                    else {
                        orientationIndex = 1;
                        }
                    }
                else {
                    // y facing

                    if( dY < 0 ) {
                        orientationIndex = 2;
                        }
                    else {
                        orientationIndex = 0;
                        }
                    }
                }
            }
        else {
            // not player-facing
            // orient based on structures and walls

            char completelyBlocked;
            
            // first, look for direction to face based on holes
            // in structural objects
            orientationIndex = 
                computeBlockedOrientation( inFullIndex,
                                           structural,
                                           &completelyBlocked );
            if( completelyBlocked ) {
                // completely surrounded by strutural objects
                // defer to walls instead.
                // (for example, if we have 3 wall and 1 non-wall neighbor,
                //  we should face out through the non-wall neighbor)
                orientationIndex = 
                    computeBlockedOrientation( inFullIndex,
                                               wall,
                                               &completelyBlocked );
                }
            }
        

        }
    else if( numOrientations == 2 ) {
        
        // double-walls trump other structural features
        if( getTileNeighborHasProperty( inFullIndex, 0, wall ) && 
            getTileNeighborHasProperty( inFullIndex, 2, wall ) ) {
            // walls on top and bottom
                
            // vertical orientation
            orientationIndex = 0;
            }
        else if( getTileNeighborHasProperty( inFullIndex, 1, wall ) && 
                 getTileNeighborHasProperty( inFullIndex, 3, wall ) ) {
            /// walls on left and right
            // horizontal 
            orientationIndex = 1;
            }
        else if( getTileNeighborHasProperty( inFullIndex, 0, wall ) || 
                 getTileNeighborHasProperty( inFullIndex, 2, wall ) ) {
            // top OR bottom wall
            
            // vertical orientation
            orientationIndex = 0;
            }
        else if( getTileNeighborHasProperty( inFullIndex, 1, wall ) || 
                 getTileNeighborHasProperty( inFullIndex, 3, wall ) ) {
            /// wall on left OR right
            // horizontal 
            orientationIndex = 1;
            }

        // no double walls on either pair of opposite sides
        // revert to reacting to any neighboring structural features
        else if( getTileNeighborHasProperty( inFullIndex, 0, structural ) && 
            getTileNeighborHasProperty( inFullIndex, 2, structural ) ) {
            // blocked on top and bottom
                
            // vertical orientation
            orientationIndex = 0;
            }
        else if( getTileNeighborHasProperty( inFullIndex, 1, structural ) && 
                 getTileNeighborHasProperty( inFullIndex, 3, structural ) ) {
            /// blocked on left and right
            // horizontal 
            orientationIndex = 1;
            }
        else if( getTileNeighborHasProperty( inFullIndex, 0, structural ) || 
                 getTileNeighborHasProperty( inFullIndex, 2, structural ) ) {
            // top OR bottom block
            
            // vertical orientation
            orientationIndex = 0;
            }
        else if( getTileNeighborHasProperty( inFullIndex, 1, structural ) || 
                 getTileNeighborHasProperty( inFullIndex, 3, structural ) ) {
            /// blocked on left OR right
            // horizontal 
            orientationIndex = 1;
            }
        else {
            // default when not blocked at all, horizontal
            orientationIndex = 1;
            }    
        }
    else if( numOrientations == 1 ) {
        orientationIndex = 0;
        }

    return orientationIndex;
    }




void HouseGridDisplay::drawDropShadow( doublePair inPosition, float inFade ) {
    setDrawColor( 0, 0, 0, inFade );
    
    toggleLinearMagFilter( true );

    drawSprite( sDropShadowSprite, inPosition, 1.0/16.0 );
    
    toggleLinearMagFilter( false );
    }




void HouseGridDisplay::drawDarkHaloBehind( int inObjectID, int inOrientation,
                                           int inState, 
                                           doublePair inPosition,
                                           float inFade ) {
    
    setDrawColor( 1, 1, 1, 0.5 * inFade );

    SpriteHandle haloSprite = 
        getObjectHaloSprite( inObjectID, 
                             inOrientation, 
                             inState );
    
    toggleLinearMagFilter( true );

    drawSprite( haloSprite, inPosition, 1.0/32.0 );

    toggleLinearMagFilter( false );
    }



void HouseGridDisplay::drawRobber( doublePair inPosition ) {
    if( mHideRobber ) {
        return;
        }

    if( ! isPropertySet( PLAYER_ID, mRobberState, noDropShadow ) ) {
        // first drop shadow
        drawDropShadow( inPosition );
        }
        
    
    if( isPropertySet( PLAYER_ID, mRobberState, darkHaloBehind ) ) {
        drawDarkHaloBehind( PLAYER_ID, 
                            mRobberOrientation, 
                            mRobberState,
                            inPosition );
        }

    setDrawColor( 1, 1, 1, 1 );

    SpriteHandle sprite = 
        getObjectSprite( PLAYER_ID, 
                         mRobberOrientation, 
                         mRobberState );
    
    drawSprite( sprite, inPosition, 1.0/32.0 );
    }





void HouseGridDisplay::drawTiles( char inBeneathShadowsOnly ) {
    for( int y=HOUSE_D-1; y>=0; y-- ) {
        for( int x=0; x<HOUSE_D; x++ ) {

            int i = y * HOUSE_D + x;
            int fullI = subToFull( i );

            int houseTile = mHouseSubMapIDs[i];
            int houseTileState = mHouseSubMapCellStates[i];
            
            
            float houseTileFade = mHouseMapCellFades[fullI];            
            
            char isNeverFade = isSubMapPropertySet( i, neverFade );

            if( isNeverFade ) {
                houseTileFade = 1;
                }

            
            char touched = mHouseMapSpotsTouched[fullI];
            
            if( ! mTouchedHighlightOn || 
                mTouchedHighlightFade == 0 ) {
                touched = false;
                }

            char aboveShadows = 
                isSubMapPropertySet( i, structural ) ||
                isSubMapPropertySet( i, shadowMaking );
                     
            char deadFamilyObject =
                isSubMapPropertySet( i, deadFamily );

            char familyObject = 
                isSubMapPropertySet( i, family ) ||
                deadFamilyObject;
            
            char forceUnderShadowsObject = 
                isSubMapPropertySet( i, forceUnderShadows ) ||
                deadFamilyObject;
            
            if( forceUnderShadowsObject ) {
                aboveShadows = false;
                }
            

            doublePair tilePos = getTilePos( i );
 

            if( inBeneathShadowsOnly && aboveShadows ) {
                // skip this blocking tile
                
                // but draw any parts under shadows
                if( isUnderSpritePresent( houseTile, 
                                          houseTileState ) ) {
                    
                    int orientationIndex = 
                        getOrientationIndex( fullI, 
                                             houseTile, houseTileState );
                    
                    setDrawColor( 1, 1, 1, houseTileFade );
                    
                    
                    SpriteHandle sprite = 
                        getObjectSpriteUnder( houseTile, 
                                               orientationIndex, 
                                               houseTileState );
                
                    drawSprite( sprite, tilePos, 1.0/32.0 );

                    if( touched ) {
                        float fade = 0.75 * mTouchedHighlightFade;
                    
                        if( mTouchedHighlightRed ) {
                            setDrawColor( 1, 0, 0, fade );
                            }
                        else {
                            setDrawColor( 0.06666, 0.68627451, 0.454901961, 
                                          fade );
                            }
                        
                        drawSpriteAlphaOnly( sprite, tilePos, 1.0/32.0 );
                        }
                    }
                

                if( mHighlightIndex != i && fullI != mRobberIndex 
                    && mHouseMapMobileIDs[ fullI ] == 0
                    && ! familyObject ) {
                    // nothing left to draw, if no highlight is here
                    // no mobile or family object is here
                    i++;
                    continue;
                    }
                }
            else if( ! inBeneathShadowsOnly && ! aboveShadows ) {
                // skip this non-blocking tile

                if( mHighlightIndex != i && fullI != mRobberIndex 
                    && mHouseMapMobileIDs[ fullI ] == 0
                    && ! familyObject ) {
                    // nothing left to draw, if no highlight is here
                    // no mobile or family object is here
                    i++;
                    continue;
                    }
                }
            

            int orientationIndex = getOrientationIndex( fullI, houseTile,
                                                        houseTileState );



            // draw "behind" parts of structural tiles before drawing
            // a robber or mobile object in this row
            if( !inBeneathShadowsOnly && aboveShadows 
                && houseTile != 0 ) {
                
                if( isBehindSpritePresent( houseTile, 
                                           houseTileState ) ) {
                    
                    
                    setDrawColor( 1, 1, 1, houseTileFade );
                    
                    
                    SpriteHandle sprite = 
                        getObjectSpriteBehind( houseTile, 
                                               orientationIndex, 
                                               houseTileState );
                
                    drawSprite( sprite, tilePos, 1.0/32.0 );

                    if( touched ) {
                        float fade = 0.75 * mTouchedHighlightFade;
                        
                        if( mTouchedHighlightRed ) {
                            setDrawColor( 1, 0, 0, fade );
                            }
                        else {
                            setDrawColor( 0.06666, 0.68627451, 0.454901961, 
                                          fade );
                            }
                        
                        drawSpriteAlphaOnly( sprite, tilePos, 1.0/32.0 );
                        }
                    }
                }
            

            // also draw beneath shadow tiles under robber or mobile
            // even if robber/mobile is under shadows itself (robber/mobile
            // is always on top of wires, switches, etc.)
            
            if( inBeneathShadowsOnly && 
                ! aboveShadows &&
                ! familyObject &&
                houseTile != 0 ) {
                
                // now draw tile itself, on top of floor

                if( isSubMapPropertySet( i, darkHaloBehind ) ) {
                    
                    drawDarkHaloBehind( houseTile, orientationIndex,
                                        houseTileState,
                                        tilePos, houseTileFade );
                    }

                if( isSubMapPropertySet( i, mobile ) &&
                    ! isSubMapPropertySet( i, noDropShadow ) ) {
                    // drop shadow
                    drawDropShadow( tilePos, houseTileFade );
                    }
                

                setDrawColor( 1, 1, 1, houseTileFade );
                
                SpriteHandle sprite = getObjectSprite( houseTile, 
                                                       orientationIndex, 
                                                       houseTileState );
                
                drawSprite( sprite, tilePos, 1.0/32.0 );

                if( touched ) {
                    float fade = 0.75 * mTouchedHighlightFade;
                    
                    if( mTouchedHighlightRed ) {
                        setDrawColor( 1, 0, 0, fade );
                        }
                    else {
                        setDrawColor( 0.06666, 0.68627451, 0.454901961, fade );
                        }
                    
                    drawSpriteAlphaOnly( sprite, tilePos, 1.0/32.0 );
                    }
                }




            char robberDrawn = false;

            // mobile objects above shadows (unless forced under, 
            // behind structural tiles in current row only
            
                
            if( mHouseMapMobileIDs[fullI] != 0 ) {
                // mobile object here
                    
                    

                int mobID = mHouseMapMobileIDs[fullI];
                int mobState = mHouseMapMobileCellStates[fullI];
                
                
                // all stuck mobiles are dead and under shadows too
                char mobForcedUnderShadows = 
                    isPropertySet( mobID, mobState, stuck )
                    ||
                    isPropertySet( mobID, mobState, forceUnderShadows );
                

                if( ( ! inBeneathShadowsOnly && ! mobForcedUnderShadows )
                    ||
                    ( inBeneathShadowsOnly && mobForcedUnderShadows ) ) {

                    int mobOrientation = getOrientationIndex( fullI, mobID,
                                                              mobState );
                    
                    // mobiles are never walls, always fade
                    float fade = mHouseMapMobileCellFades[fullI];
                    
                    if( isPropertySet( mobID, mobState, darkHaloBehind ) ) {
                        
                        drawDarkHaloBehind( mobID, mobOrientation, mobState,
                                            tilePos, fade );
                        }

                    if( ! isPropertySet( mobID, mobState, noDropShadow ) ) {
                        // first drop shadow
                        drawDropShadow( tilePos, fade );
                        }
                        
                    setDrawColor( 1, 1, 1, fade );
                
                    SpriteHandle sprite = 
                        getObjectSprite( mobID, 
                                         mobOrientation, 
                                         mobState );
                
                    drawSprite( sprite, tilePos, 1.0/32.0 );

                    if( touched ) {
                        float fade = 0.75 * mTouchedHighlightFade;
                        
                        if( mTouchedHighlightRed ) {
                            setDrawColor( 1, 0, 0, fade );
                            }
                        else {
                            setDrawColor( 0.06666, 0.68627451, 0.454901961, 
                                          fade );
                            }
                        
                        drawSpriteAlphaOnly( sprite, tilePos, 1.0/32.0 );
                        }
                    }
                }
            

            // family objects draw like mobiles, but they're not
            // (because they can co-occupy a space with a mobile,
            //  and they move differently)
            if( familyObject ) {
                
                if( ( ! inBeneathShadowsOnly && ! forceUnderShadowsObject )
                    ||
                    ( inBeneathShadowsOnly && forceUnderShadowsObject ) ) {  
                    
                    // draw now

                    if( isPropertySet( houseTile, houseTileState, 
                                       darkHaloBehind ) ) {
                        
                        drawDarkHaloBehind( houseTile, orientationIndex,
                                            houseTileState,
                                            tilePos,
                                            houseTileFade );
                        }
                        
                    drawDropShadow( tilePos, houseTileFade );
                    
                    setDrawColor( 1, 1, 1, houseTileFade );
                    
                    SpriteHandle sprite = getObjectSprite( houseTile, 
                                                           orientationIndex, 
                                                           houseTileState );
                    
                    drawSprite( sprite, tilePos, 1.0/32.0 );
                    }
                }


            // same for robber (if not already drawn under a mobile)
            if( !inBeneathShadowsOnly && mRobberIndex == fullI
                &&
                ! robberDrawn ) {

                if( ! isPropertySet( PLAYER_ID, 
                                     mRobberState, forceUnderShadows ) ) {
                    drawRobber( tilePos );
                    }
                }
            else if( inBeneathShadowsOnly && mRobberIndex == fullI 
                     &&
                     ! robberDrawn ) {
                if( isPropertySet( PLAYER_ID, 
                                   mRobberState, forceUnderShadows ) ) {
                    drawRobber( tilePos );
                    }
                }
            
                
                



            if( !inBeneathShadowsOnly && aboveShadows 
                     && houseTile != 0 ) {
                // now draw blocking objects on top of floor
                
                if( isSubMapPropertySet( i, darkHaloBehind ) ) {
                    
                    drawDarkHaloBehind( houseTile, orientationIndex,
                                        houseTileState,
                                        tilePos, houseTileFade );
                    }


                setDrawColor( 1, 1, 1, houseTileFade );
                

                SpriteHandle sprite = getObjectSprite( houseTile, 
                                                       orientationIndex, 
                                                       houseTileState );
                
                drawSprite( sprite, tilePos, 1.0/32.0 );

                if( touched ) {
                    float fade = 0.75 * mTouchedHighlightFade;
                    
                    if( mTouchedHighlightRed ) {
                        setDrawColor( 1, 0, 0, fade );
                        }
                    else {
                        setDrawColor( 0.06666, 0.68627451, 0.454901961, fade );
                        }
                    
                    drawSpriteAlphaOnly( sprite, tilePos, 1.0/32.0 );
                    }
                }

            

            // draw target highlights on top of tiles or mobile objects
            if( inBeneathShadowsOnly && mHouseMapMobileIDs[fullI] != 0 ) {
                // don't draw any target highlights here
                // because there's a mobile here that will be drawn later
                // with any target highlight on top

                }
            else if( mToolTargetFullIndices.getElementIndex( fullI ) != -1 ) {
                
                // look at tile to the south
                // if it's blocking, draw a half-strength highlight here
                // (because we're going to overlay the other half-strength
                //  layer later, after southern blocking tile is drawn).
                // But if southern tile is not blocking, draw full-strength
                // now (because no additional overlay is coming later

                char southBlocking = true;
                
                int southI = fullI - mFullMapD;

                if( southI >= 0 ) {
                    southBlocking = 
                        isPropertySet( mHouseMapIDs[ southI ],
                                       mHouseMapCellStates[ southI ],
                                       blocking );
                    }
                
                double fade = 0.5;
                
                if( southBlocking ) {
                    fade = 0.25;
                    }

                if( fullI == mToolTargetPickedFullIndex ) {
                    setDrawColor( 1, 0, 0, fade );
                    }
                else {
                    setDrawColor( 1, 1, 1, fade );
                    }

                drawSprite( mToolTargetSprite, tilePos, 
                            1.0 / 16.0 );

                setDrawColor( 1, 1, 1, 1 );
                drawSprite( mToolTargetBorderSprite, tilePos, 
                            1.0 / 16.0 );
                }
            else if( fullI != mStartIndex && // handle start separately
                     mSpecialHighlightFullIndices.getElementIndex( fullI ) 
                     != -1 ) {
                
                int highlightIndex = 
                    mSpecialHighlightFullIndices.getElementIndex( fullI );
                
                SpriteHandle highlightSprite = 
                    *( mSpecialHighlightSprites.getElement( highlightIndex ) );
                
                
                // look at tile to the south
                // if it's blocking, skip drawing entirely
                // (draw it on top later)
                // This is different from tool target highligh, where we
                // draw half strength on top of blocking tiles later

                char southBlocking = true;
                
                int southI = fullI - mFullMapD;

                if( southI >= 0 ) {
                    southBlocking = 
                        isPropertySet( mHouseMapIDs[ southI ],
                                       mHouseMapCellStates[ southI ],
                                       blocking );
                    }
                
                if( !southBlocking ) {
                    setDrawColor( 1, 1, 1, 0.5 );
                    
                    drawSprite( highlightSprite, tilePos, 1.0 / 16.0 );
                    }
                }
            

            
            

            
            // no highlight over start, robber, or permanents
            int highlightPick = -1;
            
            if( mHighlightIndex == i &&
                fullI != mStartIndex &&
                fullI != mRobberIndex &&
                ! isSubMapPropertySet( i, permanent ) ) {
                
                if( mPicker != NULL ) {
                    highlightPick = mPicker->getSelectedObject();
                    }
                }


            if( highlightPick != -1 ) {
                int mobID = mHouseMapMobileIDs[fullI];
                
                if( mMandatoryNeedsPlacing ) {
                    // ghost of to-place mandatory for placement
                    setDrawColor( 1, 1, 1, 0.35 );

                    int ghostOrientation = 
                        getOrientationIndex( fullI, 
                                             mMandatoryToPlaceID,
                                             0 );
                    
                    SpriteHandle sprite = getObjectSprite( 
                        mMandatoryToPlaceID, 
                        ghostOrientation, 
                        0 );
                
                    drawSprite( sprite, tilePos, 1.0/32.0 );
                    }
                else if( 
                    // placing empty floor, and current tile not empty floor
                    ( highlightPick == 0 && houseTile != 0 ) ||
                    // not placing empty, and current tile matches placement
                    // (erase-same behavior)
                    ( highlightPick != 0 && 
                      houseTile == highlightPick ) ||
                    // mobile here
                    // placing empty floor or erase mode
                    ( mobID != 0 &&
                      ( highlightPick == 0 || highlightPick == mobID ) ) ||
                    // family here (clicking always removes them)
                    isSubMapPropertySet( i, family ) ||
                    // vault here (clicking always removes it)
                    houseTile == GOAL_ID ) {

                    


                    // darken existing tile to imply removal on click
                    setDrawColor( 0, 0, 0, 0.35 );
                    
                    SpriteHandle sprite;

                    if( isUnderSpritePresent( houseTile, houseTileState ) ) {
                        sprite = getObjectSpriteUnder( houseTile, 
                                                       orientationIndex,
                                                       houseTileState );
                        drawSprite( sprite, tilePos, 1.0/32.0 );
                        }
                    if( isBehindSpritePresent( houseTile, houseTileState ) ) {
                        sprite = getObjectSpriteBehind( houseTile, 
                                                        orientationIndex,
                                                        houseTileState );
                        drawSprite( sprite, tilePos, 1.0/32.0 );
                        }
                    
                    
                    if( houseTile != 0 ) {
                        
                        sprite = getObjectSprite( houseTile, 
                                                  orientationIndex, 
                                                  houseTileState );
                        
                        drawSprite( sprite, tilePos, 1.0/32.0 );
                        }

                    
                    if( mobID != 0 ) {
                        int mobState = mHouseMapMobileCellStates[fullI];
                        
                        int mobOrientation = getOrientationIndex( fullI, mobID,
                                                                  mobState );

                        sprite = getObjectSprite( mobID, 
                                                  mobOrientation, 
                                                  mobState);
                                                  
                        drawSprite( sprite, tilePos, 1.0/32.0 );
                        }                    

                    }
                else if( houseTile != highlightPick ) {
                    setDrawColor( 1, 1, 1, 0.35 );
                
                    int ghostOrientation = getOrientationIndex( fullI, 
                                                                highlightPick, 
                                                                0 );

                    SpriteHandle sprite;


                    if( isUnderSpritePresent( highlightPick, 0 ) ) {
                        sprite = getObjectSpriteUnder( highlightPick, 
                                                       ghostOrientation,
                                                       0 );
                        drawSprite( sprite, tilePos, 1.0/32.0 );
                        }
                    if( isBehindSpritePresent( highlightPick, 0 ) ) {
                        sprite = getObjectSpriteBehind( highlightPick, 
                                                       ghostOrientation,
                                                       0 );
                        drawSprite( sprite, tilePos, 1.0/32.0 );
                        }


                    sprite = getObjectSprite( highlightPick, 
                                              ghostOrientation,
                                              0 );
                    
                    drawSprite( sprite, tilePos, 1.0/32.0 );
                    }
                }

            i++;
            }
        }
    }






void HouseGridDisplay::draw() {
    if( !mHouseMapSet ) {
        return;
        }

    
    // draw border
    doublePair center = { 0, 0 };
    
    setDrawColor( 1, 1, 1, 1 );
    drawSquare( center, HOUSE_D * mTileRadius + .125 );

    
    enableScissor( -HOUSE_D * mTileRadius,
                   -HOUSE_D * mTileRadius,
                   2 * ( HOUSE_D * mTileRadius ),
                   2 * ( HOUSE_D * mTileRadius ) );

    // draw full grid of floor under everything
    SpriteHandle sprite = getObjectSprite( 0, 0, mLastResetTargetState );
    setDrawColor( 1, 1, 1, 1 );
    for( int y=HOUSE_D-1; y>=0; y-- ) {
        for( int x=0; x<HOUSE_D; x++ ) {
            drawSprite( sprite, getTilePos( y * HOUSE_D + x ), 1.0/32.0 );
            }
        }
    


    // first, walk along top edge and draw one row of off-screen tiles, if 
    // present (this "completes" top visible row of tiles by drawing
    // overhangs from tiles that are north of them)

    if( mSubMapOffsetY + HOUSE_D < mFullMapD ) {
        
        int y = mSubMapOffsetY + HOUSE_D;
        
        for( int x=0; x<HOUSE_D; x++ ) {    
        
            int fullI = y * mFullMapD + mSubMapOffsetX + x;
                
            int houseTile = mHouseMapIDs[ fullI ];
            int houseTileState = mHouseMapCellStates[ fullI ];

            int orientationIndex = getOrientationIndex( fullI, houseTile,
                                                        houseTileState );

            doublePair tilePos = getTilePosFull( fullI );

            setDrawColor( 1, 1, 1, 1 );
                

            SpriteHandle sprite = getObjectSprite( houseTile, 
                                                   orientationIndex, 
                                                   houseTileState );
            
            drawSprite( sprite, tilePos, 1.0/32.0 );
            }
        }    

    
    // draw house parts that are under shadows (non-structural parts)
    drawTiles( true );

    
    // draw start under shadows
    int startSubIndex = fullToSub( mStartIndex );
    if( startSubIndex != -1 ) {
        setDrawColor( 1, 1, 1, 1 );

        SpriteHandle sprite = getObjectSprite( START_ID, 
                                               0, 
                                               mLastResetTargetState );
        drawSprite( sprite, getTilePos( startSubIndex ), 1.0/32.0 );
        }


    
    // walk along bottom edge and draw one last row of tiles, if 
    // present (this "completes" bottom visible row of tiles by drawing
    // overhangs from tiles that are south of them)
    // 
    // here we do this for tiles in bottom row that are forced under shadows

    if( mSubMapOffsetY > 0 ) {
        
        int y = mSubMapOffsetY - 1;
        
        for( int x=0; x<HOUSE_D; x++ ) {    
        
            int fullI = y * mFullMapD + mSubMapOffsetX + x;
              
            if( isMapPropertySet( fullI, forceUnderShadows ) ) {
                
                int houseTile = mHouseMapIDs[ fullI ];
                int houseTileState = mHouseMapCellStates[ fullI ];
                
                int numOrientations = getNumOrientations( houseTile, 0 );
            
                if( numOrientations == 16 // connected object
                    || // or bottom row unfaded
                    ! mBottomRowNonConnectedFaded ) { 

                    int orientationIndex = 
                        getOrientationIndex( fullI, 
                                             houseTile,
                                             houseTileState );

                    doublePair tilePos = getTilePosFull( fullI );
                    
                    if( isPropertySet( houseTile, houseTileState, 
                                       darkHaloBehind ) ) {
                        
                        drawDarkHaloBehind( houseTile, orientationIndex,
                                            houseTileState,
                                            tilePos,
                                            1 );
                        }
                    
                    setDrawColor( 1, 1, 1, 1 );
                    
                    
                    SpriteHandle sprite = getObjectSprite( houseTile, 
                                                           orientationIndex, 
                                                           houseTileState );
                    
                    drawSprite( sprite, tilePos, 1.0/32.0 );
                    }
                }
            }
        }




    
    setDrawColor( 0, 0, 0, 0.75 );

    toggleLinearMagFilter( true );
    // draw wall shadows over floor
    drawSprite( mWallShadowSprite, center, 
                1.0 * 2 * mTileRadius / 8.0 );
    toggleLinearMagFilter( false );

    



    // draw structural tiles above shadows
    drawTiles( false );



    // finally, walk along bottom edge and draw one last row of tiles, if 
    // present (this "completes" bottom visible row of tiles by drawing
    // overhangs from tiles that are south of them)
    //
    // here we do this for tiles in bottom row that are NOT 
    // forced under shadows

    if( mSubMapOffsetY > 0 ) {
        
        int y = mSubMapOffsetY - 1;
        
        for( int x=0; x<HOUSE_D; x++ ) {    
        
            int fullI = y * mFullMapD + mSubMapOffsetX + x;
              
            if( ! isMapPropertySet( fullI, forceUnderShadows ) ) {
                
                int houseTile = mHouseMapIDs[ fullI ];
                int houseTileState = mHouseMapCellStates[ fullI ];
                
                int numOrientations = getNumOrientations( houseTile, 0 );
            
                if( numOrientations == 16 // connected object
                    || // or bottom row unfaded
                    ! mBottomRowNonConnectedFaded ) { 

                
                    int orientationIndex = 
                        getOrientationIndex( fullI, houseTile,
                                             houseTileState );
                    
                    doublePair tilePos = getTilePosFull( fullI );
                    
                    if( isPropertySet( houseTile, houseTileState, 
                                       darkHaloBehind ) ) {
                        
                        drawDarkHaloBehind( houseTile, orientationIndex,
                                            houseTileState,
                                            tilePos,
                                            1 );
                        }
                
                    setDrawColor( 1, 1, 1, 1 );
                
                
                    SpriteHandle sprite = getObjectSprite( houseTile, 
                                                           orientationIndex, 
                                                           houseTileState );
                    
                    drawSprite( sprite, tilePos, 1.0/32.0 );
                    }
                }
            }
        }




    // draw target highlights one more time, faint
    // so they "show through" overlapping tiles and objects a bit
    // only do this if south tile below highlight is blocking

    for( int i=0; i<mToolTargetFullIndices.size(); i++ ) {
        int fullI = *( mToolTargetFullIndices.getElement(i) );
        
        int subI = fullToSub( fullI );
        
        if( subI != -1 ) {
            char southBlocking = true;
        
            int southI = fullI - mFullMapD;
            
            if( southI >= 0 ) {
                southBlocking = 
                    isPropertySet( mHouseMapIDs[ southI ],
                                   mHouseMapCellStates[ southI ],
                                   blocking );
                }

            if( southBlocking ) {
                
                doublePair tilePos = getTilePos( subI );
                
                if( fullI == mToolTargetPickedFullIndex ) {
                    setDrawColor( 1, 0, 0, 0.25 );
                    }
                else {
                    setDrawColor( 1, 1, 1, 0.25 );
                    }

                drawSprite( mToolTargetSprite, tilePos, 
                            1.0 / 16.0 );
                
                setDrawColor( 1, 1, 1, 0.5 );
                drawSprite( mToolTargetBorderSprite, tilePos, 
                            1.0 / 16.0 );
                }
            }
        }

    for( int i=0; i<mSpecialHighlightFullIndices.size(); i++ ) {
        int fullI = *( mSpecialHighlightFullIndices.getElement(i) );
        
        int subI = fullToSub( fullI );
        
        if( subI != -1 ) {
            char southBlocking = true;
        
            int southI = fullI - mFullMapD;
            
            if( southI >= 0 ) {
                southBlocking = 
                    isPropertySet( mHouseMapIDs[ southI ],
                                   mHouseMapCellStates[ southI ],
                                   blocking );
                }

            if( southBlocking ) {
                // draw full strength
                doublePair tilePos = getTilePos( subI );
                
                setDrawColor( 1, 1, 1, 0.5 );
                
                SpriteHandle highlightSprite = 
                    *( mSpecialHighlightSprites.getElement(i) );

                drawSprite( highlightSprite, tilePos, 1.0 / 16.0 );
                }
            }
        }
    


    
    // noise over everything
    int i = 0;
    for( int y=0; y<HOUSE_D; y++ ) {
        for( int x=0; x<HOUSE_D; x++ ) {
            int fullI = subToFull( i );

            doublePair tilePos = getTilePos( i );

            // pencil scratches wherever tool tips have been overridden
            if( mHouseMapToolTipOverrideOn[ fullI ] ||
                mHouseMapMobileToolTipOverrideOn[ fullI ] ) {
                
                setDrawColor( 1, 1, 1, 1 );
                
                drawSprite( 
                    sPencilScratchBank[ mHouseMapNoiseTileIndices[fullI] ],
                    tilePos, 1.0/16.0 );
                }
            

            setDrawColor( 1, 1, 1, 0.0625 );
            drawSprite( sNoiseTileBank[ mHouseMapNoiseTileIndices[fullI] ],
                        tilePos, 1.0/16.0 );
            
            i++;
            }
        }
            

    

    

    disableScissor();
    }



void HouseGridDisplay::pointerOver( float inX, float inY ) {
    mHighlightIndex = getTileIndex( inX, inY );

    if( mHighlightIndex != -1 ) {
        mPointerInside = true;
        
        int fullI = subToFull( mHighlightIndex );
        
        
        char *nonMobileDescription; 

        if( fullI == mStartIndex && mHouseMapIDs[fullI] == 0 ) {
            nonMobileDescription = 
                stringDuplicate( translate( "startTileDescription" ) );
            }
        else {                
            int id = mHouseSubMapIDs[ mHighlightIndex ];
            int state = mHouseSubMapCellStates[ mHighlightIndex ];

            
            
            if( mHouseMapToolTipOverrideOn[ fullI ] ) {
                // override state for picking tool tip
                state = mHouseMapToolTipOverrideState[ fullI ];
                }
            

            nonMobileDescription = getObjectDescription( id, state, 
                                                         mWifeName );
            
            if( isPropertySet( id, state, wife ) && mWifeName != NULL ) {

                char *nameInserted =
                    autoSprintf( nonMobileDescription, mWifeName );
                
                delete [] nonMobileDescription;
                nonMobileDescription = nameInserted;
                }
            if( isPropertySet( id, state, son ) && mSonName != NULL ) {
                
                char *nameInserted =
                    autoSprintf( nonMobileDescription, mSonName );
                
                delete [] nonMobileDescription;
                nonMobileDescription = nameInserted;
                }
            if( isPropertySet( id, state, daughter ) && 
                mDaughterName != NULL ) {

                char *nameInserted =
                    autoSprintf( nonMobileDescription, mDaughterName );
                
                delete [] nonMobileDescription;
                nonMobileDescription = nameInserted;
                }
            


            if( mWifeMoney > 0 && 
                isPropertySet( id, state, wife ) ) {

                char *fullDescription =
                    autoSprintf( translate( "wifeHolding" ), 
                                 nonMobileDescription, mWifeMoney );
                
                delete [] nonMobileDescription;
                nonMobileDescription = fullDescription;
                }
            }
        

        if( mHouseMapMobileIDs[ fullI ] != 0 ) {
            
            int id = mHouseMapMobileIDs[ fullI ];
            int state = mHouseMapMobileCellStates[ fullI ];
            
            if( mHouseMapMobileToolTipOverrideOn[ fullI ] ) {
                // override state for picking tool tip
                state = mHouseMapMobileToolTipOverrideState[ fullI ];
                }

            char *mobileDescription = getObjectDescription( id, state, 
                                                            mWifeName );
            
            if( mHouseMapIDs[fullI] != 0 ) {
                // mobile on top of something
                char *tip = autoSprintf( "%s  /  %s",
                                         nonMobileDescription, 
                                         mobileDescription );
                setToolTip( tip );
                
                delete [] tip;
                }
            else {
                // don't describe empty floor under mobile
                setToolTip( mobileDescription );
                }

            delete [] mobileDescription;
            }
        else {
            setToolTip( nonMobileDescription );
            }
        
        delete [] nonMobileDescription;
        }
    else if( mPointerInside ) {
        // just moved off grid
        setToolTip( NULL );
        
        mPointerInside = false;
        }
    }



void HouseGridDisplay::placeMandatory( int inFullIndex, int inIndex ) {
    
    int oldID = mHouseSubMapIDs[ inIndex ];
    int oldState = mHouseSubMapCellStates[ inIndex ];
    
    mHouseSubMapIDs[ inIndex ] = mMandatoryToPlaceID;
    
    logEdit( inFullIndex, mMandatoryToPlaceID );

    
    if( mMandatoryToPlaceID == GOAL_ID ) {
        mGoalIndex = inFullIndex;
        }
    if( isPropertySet( oldID, oldState, mandatory ) ) {
        mMandatoryToPlaceID = oldID;
        }
    else {
        mMandatoryNeedsPlacing = false;
        }
    
    mLastPlacedObject = mMandatoryToPlaceID;    
    
    
    // changes reset state
    mHouseSubMapCellStates[ inIndex ] = 0;
    
    // clear mobile objects
    mHouseMapMobileIDs[ inFullIndex ] = 0;
    mHouseMapMobileCellStates[ inFullIndex ] = 0;
    
    copySubCellBack( inIndex );

    // placing a mandatory can remove another object, so it might
    // change the diff
    mLastActionChangedDiff = true;
    mLastActionPlayerMotion = false;
    fireActionPerformed( this );
    }




void HouseGridDisplay::pointerMove( float inX, float inY ) {
    pointerOver( inX, inY );
    }

void HouseGridDisplay::pointerUp( float inX, float inY ) {
    pointerOver( inX, inY );

    int index = getTileIndex( inX, inY );
    
    mHighlightIndex = index;
    
    if( index != -1 && 
        mPointerDownIndex != -1 &&
        mDraggedAway &&
        ! isSubMapPropertySet( index, permanent ) ) {
    
        // pointer dropped on grid after being dragged somewhere else
        int fullIndex = subToFull( index );
        
        
        if( mMandatoryNeedsPlacing && 
            fullIndex != mStartIndex &&
            fullIndex != mRobberIndex ) {
            // mandatory dragged/dropped here

            placeMandatory( fullIndex, index );
            }

        }
    
    }



void HouseGridDisplay::pointerDrag( float inX, float inY ) {
    pointerOver( inX, inY );
    int index = getTileIndex( inX, inY );

    mHighlightIndex = index;

    if( index != -1 && 
        mPointerDownIndex != -1 &&
        mPointerDownObjectID != -1 &&
        ! isSubMapPropertySet( index, permanent ) &&
        ! isSubMapPropertySet( index, mandatory ) ) {

        mDraggedAway = true;

        int fullIndex = subToFull( index );

        if( fullIndex != mRobberIndex &&
            fullIndex != mStartIndex ) {
            
            int old = mHouseSubMapIDs[ index ];
            int oldMobile = mHouseMapMobileIDs[ fullIndex ];

            // placement of floor is always drag-place
            if( mPlaceOnDrag &&
                ( mPointerDownObjectID == 0 
                  ||
                  ( old != mPointerDownObjectID &&
                    oldMobile != mPointerDownObjectID ) ) ) {
                
                // drag-place
                if( mAllowPlacement ) {
                    mHouseSubMapIDs[ index ] = mPointerDownObjectID;
                    mLastPlacedObject = mPointerDownObjectID;

                    // avoid logging floor-on-floor placement as edits
                    if( old != mPointerDownObjectID || 
                        oldMobile != mPointerDownObjectID ) {

                        logEdit( fullIndex, mPointerDownObjectID );
                        }
                    
                    // changes reset state
                    mHouseSubMapCellStates[ index ] = 0;

                    // clear mobile objects
                    mHouseMapMobileIDs[ fullIndex ] = 0;
                    mHouseMapMobileCellStates[ fullIndex ] = 0;
                    
                    copySubCellBack( index );
                    
                    mLastActionChangedDiff = true;
                    mLastActionPlayerMotion = false;
                    fireActionPerformed( this );
                    }
                }
            else if( !mPlaceOnDrag && 
                     ( old == mPointerDownObjectID ||
                       oldMobile == mPointerDownObjectID ) ) {
                
                // drag-erase of like-objects
                mHouseSubMapIDs[ index ] = 0;
                mLastPlacedObject = 0;

                logEdit( fullIndex, 0 );

                // changes reset state
                mHouseSubMapCellStates[ index ] = 0;

                // clear mobile objects
                mHouseMapMobileIDs[ fullIndex ] = 0;
                mHouseMapMobileCellStates[ fullIndex ] = 0;

                copySubCellBack( index );
                
                mLastActionChangedDiff = true;
                mLastActionPlayerMotion = false;
                fireActionPerformed( this );
                }
            
            
            }

        }
    }



void HouseGridDisplay::pointerDown( float inX, float inY ) {
    int index = getTileIndex( inX, inY );
    
    mPointerDownIndex = index;
    mDraggedAway = false;
    mPointerDownObjectID = -1;
    
    if( index == -1 ) {
        return;
        }
    
    if( isSubMapPropertySet( index, permanent ) ) {
        // ignore mouse activity on permanent tiles
        return;
        }

    int fullIndex = subToFull( index );
    printf( "pointer down at full index %d (%d,%d)\n", fullIndex,
            index % HOUSE_D, index / HOUSE_D );
    

    
    if( isCommandKeyDown() ) {
        // CTRL-click
        // "eyedropper" the tile under the mouse
        int nonMobile = mHouseMapIDs[ fullIndex ];
        int mobile = mHouseMapMobileIDs[ fullIndex ];
        
        if( mobile != 0 ) {
            mPicker->setSelectedObject( mobile );
            }
        else if( 
            !isPropertySet( nonMobile,
                            mHouseMapCellStates[ fullIndex ], 
                            mandatory ) ) {
            mPicker->setSelectedObject( nonMobile );
            }
        else {
            mPicker->setSelectedObject( 0 );
            }

        return;
        }
    

    if( fullIndex == mRobberIndex ) {
        // don't allow clicks on current robber position
        return;
        }
    

    
    

    if( mMandatoryNeedsPlacing && fullIndex != mStartIndex ) {
        placeMandatory( fullIndex, index );
        }
    else if( !mMandatoryNeedsPlacing && 
             isPropertySet( mHouseMapIDs[ fullIndex ],
                            mHouseMapCellStates[ fullIndex ], mandatory ) ) {
        // mandatory moving!
        mMandatoryToPlaceID = mHouseSubMapIDs[ index ];
        mMandatoryNeedsPlacing = true;
        
        mHouseSubMapIDs[ index ] = 0;
 
        mLastPlacedObject = 0;

        logEdit( fullIndex, 0 );

        // changes reset state
        mHouseSubMapCellStates[ index ] = 0;

        
        // Don't clear mobile objects just because mandatory picked up 
        // from there
        

        copySubCellBack( index );
        
        mLastActionChangedDiff = false;
        mLastActionPlayerMotion = false;
        fireActionPerformed( this );
        }
    else if( fullIndex != mStartIndex ) {
    
        if( mPicker != NULL ) {
            
        
            int old = mHouseSubMapIDs[ index ];
            int oldMobile = mHouseMapMobileIDs[ fullIndex ];
            
            int picked = mPicker->getSelectedObject();
            
            // floor placement always counts as place mode
            if( picked == 0 
                ||
                ( old != picked && oldMobile != picked ) ) {
                // place mode (or replace mode)
                if( mAllowPlacement ) {
                    mHouseSubMapIDs[ index ] = picked;
                    mLastPlacedObject = picked;
                    mPointerDownObjectID = picked;
                    mPlaceOnDrag = true;

                    // avoid logging floor-on-floor placement as edits
                    if( old != picked || oldMobile != picked ) {    
                        logEdit( fullIndex, picked );
                        }
                    
                    // changes reset state
                    mHouseSubMapCellStates[ index ] = 0;

                    // clear mobile objects
                    mHouseMapMobileIDs[ fullIndex ] = 0;
                    mHouseMapMobileCellStates[ fullIndex ] = 0;

                    copySubCellBack( index );
                    
                    mLastActionChangedDiff = true;
                    mLastActionPlayerMotion = false;
                    fireActionPerformed( this );
                    }
                }
            else {
                // erase mode
                mHouseSubMapIDs[ index ] = 0;
                mLastPlacedObject = 0;
                
                // only allow erase of this object ID on drag
                mPointerDownObjectID = picked;
                mPlaceOnDrag = false;
                
                logEdit( fullIndex, 0 );

                // changes reset state
                mHouseSubMapCellStates[ index ] = 0;

                // clear mobile objects
                mHouseMapMobileIDs[ fullIndex ] = 0;
                mHouseMapMobileCellStates[ fullIndex ] = 0;

                copySubCellBack( index );
                
                mLastActionChangedDiff = true;
                mLastActionPlayerMotion = false;
                fireActionPerformed( this );
                }
            }
        }
    
    pointerOver( inX, inY );
    }



extern char upKey;
extern char leftKey;
extern char downKey;
extern char rightKey;



void HouseGridDisplay::keyDown( unsigned char inASCII ) {    

    unsigned char lowerCase = tolower( inASCII );
    
    if( lowerCase == upKey ) {
        specialKeyDown( MG_KEY_UP );
        }
    else if( lowerCase == leftKey ) {
        specialKeyDown( MG_KEY_LEFT );
        }
    else if( lowerCase == downKey ) {
        specialKeyDown( MG_KEY_DOWN );
        }
    else if( lowerCase == rightKey ) {
        specialKeyDown( MG_KEY_RIGHT );
        }
    }



void HouseGridDisplay::keyUp( unsigned char inASCII ) {    

    unsigned char lowerCase = tolower( inASCII );

    if( lowerCase == upKey ) {
        specialKeyUp( MG_KEY_UP );
        }
    else if( lowerCase == leftKey ) {
        specialKeyUp( MG_KEY_LEFT );
        }
    else if( lowerCase == downKey ) {
        specialKeyUp( MG_KEY_DOWN );
        }
    else if( lowerCase == rightKey ) {
        specialKeyUp( MG_KEY_RIGHT );
        }
    }



// arrow key movement
void HouseGridDisplay::specialKeyDown( int inKeyCode ) {
    
    int oldX = mRobberIndex % mFullMapD;
    int oldY = mRobberIndex / mFullMapD;
    
    
    int newX = oldX;
    int newY = oldY;

    if( mSpecialKeysHeldSteps[ inKeyCode ] == 0 ) {
        // not already down

        mSpecialKeysHeldSteps[ inKeyCode ] = 1;
        mSpecialKeysHeldStepsTotal[ inKeyCode ] = 1;
        }

    // clear hold status of all other special keys
    for( int i=0; i<MG_KEY_LAST_CODE+1; i++ ) {
        if( i != inKeyCode ) {
            mSpecialKeysHeldSteps[i] = 0;
            mSpecialKeysHeldStepsTotal[i] = 0;
            }
        }
    

    if( inKeyCode == MG_KEY_LEFT ) {
        if( newX > 0 ) {
            newX--;
            }
        else if( mRobberIndex == mStartIndex ) {
            robberTriedToLeave();
            }
        }
    else if( inKeyCode == MG_KEY_RIGHT ) {
        if( newX < mFullMapD - 1 ) {
            newX++;
            }
        }
    else if( inKeyCode == MG_KEY_DOWN ) {
        if( newY > 0 ) {
            newY--;
            }
        }
    else if( inKeyCode == MG_KEY_UP ) {
        if( newY < mFullMapD - 1 ) {
            newY++;
            }
        }
    
    
    
    int newRobberIndex = newY * mFullMapD + newX;
    int newRobberSubIndex = fullToSub( newRobberIndex );
    
    if( newRobberIndex != mRobberIndex &&
        ! isSubMapPropertySet( newRobberSubIndex, blocking ) &&
        ! ( mRobberState == 0 && 
            isSubMapPropertySet( newRobberSubIndex, deadly ) ) ) {
        
        // did not hit wall (or deadly object, which also block in edit mode), 
        // can actually move here
        moveRobber( newRobberIndex );
        mLastPlacedObject = 0;
        
        mLastActionChangedDiff = false;
        mLastActionPlayerMotion = true;
        fireActionPerformed( this );
        }
    
    }



void HouseGridDisplay::specialKeyUp( int inKeyCode ) {
    mSpecialKeysHeldSteps[ inKeyCode ] = 0;
    mSpecialKeysHeldStepsTotal[ inKeyCode ] = 0;
    }



void HouseGridDisplay::moveRobber( int inNewIndex ) {
    int newRobberSubIndex = fullToSub( inNewIndex );

    if( newRobberSubIndex == -1 ||
        isSubMapPropertySet( newRobberSubIndex, blocking ) ) {
        // check for blocking here too so that moveRobber can be used
        // blindly by subclasses
        mForbiddenMoveHappened = true;
        return;
        }
    
    

    if( mRobberIndex != inNewIndex ) {
        
        int oldX = mRobberIndex % mFullMapD;
        int oldY = mRobberIndex / mFullMapD;
        int newX = inNewIndex % mFullMapD;
        int newY = inNewIndex / mFullMapD;
        
        int dX = newX - oldX;
        
        int dY = newY - oldY;
        
        if( ( dX != 0 && dY != 0 )
            ||
            dX > 1 
            ||
            dX < -1
            ||
            dY > 1
            ||
            dY < -1 ) {
            
            // block moves that jump more than one space
            return;
            }
        


        if( dX != 0 ) {
            if( dX == 1 ) {
                mRobberOrientation = 1;
                }
            else { 
                mRobberOrientation = 3;
                }
            }
        else if( dY != 0 ) {
            if( dY == 1 ) {
                mRobberOrientation = 0;
                }
            else { 
                mRobberOrientation = 2;
                }
            }
        

        mLastRobberIndex = mRobberIndex;
        mRobberIndex = inNewIndex;

        // if robber too close to edge, move view to keep robber on screen
        int robSubIndex = fullToSub( mRobberIndex );
        int robSubY = robSubIndex / HOUSE_D;
        int robSubX = robSubIndex % HOUSE_D;
    

        int xExtra = 0;
        int yExtra = 0;

        if( robSubX > HOUSE_D - 4 ) {
            xExtra = robSubX - (HOUSE_D - robSubX);
            xExtra /= 2;
            xExtra += 2;
            
            if( xExtra + mSubMapOffsetX + HOUSE_D > mFullMapD ) {
                xExtra = mFullMapD - ( mSubMapOffsetX + HOUSE_D );
                }
            }
        else if( robSubX < 3 ) {
            xExtra = robSubX - (HOUSE_D - robSubX);
            xExtra /= 2;
            xExtra -= 1;
            
            if( xExtra + mSubMapOffsetX < 0 ) {
                xExtra = -mSubMapOffsetX;
                }        
            }
    
        if( robSubY > HOUSE_D - 4 ) {
            yExtra = robSubY - (HOUSE_D - robSubY);
            yExtra /= 2;
            yExtra += 2;
            
            if( yExtra + mSubMapOffsetY + HOUSE_D > mFullMapD ) {
                yExtra = mFullMapD - ( mSubMapOffsetY + HOUSE_D );
                }        
            }
        else if( robSubY < 3 ) {
            yExtra = robSubY - (HOUSE_D - robSubY);
            yExtra /= 2;
            yExtra -= 1;
            
            if( yExtra + mSubMapOffsetY < 0 ) {
                yExtra = -mSubMapOffsetY;
                }        
            }


        if( xExtra != 0 || yExtra != 0 ) {
            setVisibleOffset( mSubMapOffsetX + xExtra,
                              mSubMapOffsetY + yExtra );
            }
        }
    }



char HouseGridDisplay::isSubMapPropertySet( int inSubCellIndex, 
                                            propertyID inProperty ) {
    return isPropertySet( mHouseSubMapIDs[ inSubCellIndex ],
                          mHouseSubMapCellStates[ inSubCellIndex ],
                          inProperty );
    }


char HouseGridDisplay::isMapPropertySet( int inFullCellIndex, 
                                         propertyID inProperty ) {
    return isPropertySet( mHouseMapIDs[ inFullCellIndex ],
                          mHouseMapCellStates[ inFullCellIndex ],
                          inProperty );
    }



int HouseGridDisplay::subToFull( int inSubCellIndex ) {
    int x = inSubCellIndex % HOUSE_D;
    int y = inSubCellIndex / HOUSE_D;

    int bigY = y + mSubMapOffsetY;
    int bigX = x + mSubMapOffsetX;

    return bigY * mFullMapD + bigX;
    }



int HouseGridDisplay::fullToSub( int inFullCellIndex ) {
    int bigX = inFullCellIndex % mFullMapD;
    int bigY = inFullCellIndex / mFullMapD;

    int x = bigX - mSubMapOffsetX;
    int y = bigY - mSubMapOffsetY;

    if( y >= 0 && y < HOUSE_D && 
        x >= 0 && x < HOUSE_D ) {
        return y * HOUSE_D + x;
        }
    return -1;
    }




void HouseGridDisplay::copySubCellBack( int inSubCellIndex ) {
    int bigIndex = subToFull( inSubCellIndex );

    if( isPropertySet( mHouseSubMapIDs[ inSubCellIndex ],
                       mHouseSubMapCellStates[ inSubCellIndex ],
                       mobile ) ) {
        // force all mobile objects into mobile layer as they are
        // placed
        mHouseMapMobileIDs[bigIndex] = mHouseSubMapIDs[ inSubCellIndex ];
        mHouseMapMobileCellStates[bigIndex] = 
            mHouseSubMapCellStates[ inSubCellIndex ];
        
        mHouseSubMapIDs[ inSubCellIndex ] = 0;
        mHouseSubMapCellStates[ inSubCellIndex ] = 0;
        }

    
    mHouseMapIDs[ bigIndex ] = 
        mHouseSubMapIDs[ inSubCellIndex ];
            
    mHouseMapCellStates[ bigIndex ] = 
        mHouseSubMapCellStates[ inSubCellIndex ];
    
    checkExitPaths();

    recomputeWallShadows();
    }



void HouseGridDisplay::copyAllIntoSubCells() {
    for( int y=0; y<HOUSE_D; y++ ) {
        int bigY = y + mSubMapOffsetY;

        for( int x=0; x<HOUSE_D; x++ ) {
            int bigX = x + mSubMapOffsetX;

            int subIndex = y * HOUSE_D + x;
            
            int bigIndex = bigY * mFullMapD + bigX;

            mHouseSubMapIDs[ subIndex ] = 
                mHouseMapIDs[ bigIndex ];
            
            mHouseSubMapCellStates[ subIndex ] = 
                mHouseMapCellStates[ bigIndex ];
            }
        }
    recomputeWallShadows();
    }



void HouseGridDisplay::setVisibleOffset( int inXOffset, int inYOffset ) {
    
    mSubMapOffsetX = inXOffset;
    mSubMapOffsetY = inYOffset;

    copyAllIntoSubCells();

    recomputeWallShadows();
    }



int HouseGridDisplay::getVisibleOffsetX() {
    return mSubMapOffsetX;
    }



int HouseGridDisplay::getVisibleOffsetY() {
    return mSubMapOffsetY;
    }







void HouseGridDisplay::recomputeWallShadows() {
    if( mSkipShadowComputation ) {
        return;
        }


    if( mWallShadowSprite != NULL ) {
        freeSprite( mWallShadowSprite );
        mWallShadowSprite = NULL;
        }
    



    int blowUpFactor = 8;
    int blownUpSize = HOUSE_D * blowUpFactor;

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

    memset( fullGridChannelsBlownUpAlpha, 255, numBlowupPixels );

    
    for( int y=0; y<HOUSE_D; y++ ) {
        
        for( int x=0; x<HOUSE_D; x++ ) {
            
            unsigned char alphaValue = 0;
            
            int flipY = HOUSE_D - y - 1;
            
            int subIndex = flipY * HOUSE_D + x;
            

            if( isSubMapPropertySet( subIndex, shadowMaking ) ) {
                alphaValue = 255;
                }
            
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
                        // set all border pixels to black
                        // and don't apply blur filter to those
                        fullGridChannelsBlownUpAlpha[ imageIndex ] = 255;
                        }
                    }
                }
            }
        }

    

    FastBoxBlurFilter filter2;

    for( int f=0; f<8; f++ ) {
        
        filter2.applySubRegion( fullGridChannelsBlownUpAlpha, 
                                touchIndices,
                                numTouched,
                                paddedSize, paddedSize );
        }
    
    
    // set border regions to solid black to avoid light halo
    for( int y=0; y<paddedSize; y++ ) {
        for( int x=0; x<blowUpBorder; x++ ) {
            fullGridChannelsBlownUpAlpha[ y * paddedSize + x ] = 255;
            }
        for( int x=paddedSize - blowUpBorder; x<paddedSize; x++ ) {
            fullGridChannelsBlownUpAlpha[ y * paddedSize + x ] = 255;
            }
        }
    for( int x=0; x<paddedSize; x++ ) {
        for( int y=0; y<blowUpBorder; y++ ) {
            fullGridChannelsBlownUpAlpha[ y * paddedSize + x ] = 255;
            }
        for( int y=paddedSize - blowUpBorder; y<paddedSize; y++ ) {
            fullGridChannelsBlownUpAlpha[ y * paddedSize + x ] = 255;
            }
        }
        
    mWallShadowSprite = 
        fillSpriteAlphaOnly( fullGridChannelsBlownUpAlpha, 
                             paddedSize, paddedSize );
    
    delete [] fullGridChannelsBlownUpAlpha;
    delete [] touchIndices;






    /*
    int blowUpFactor = 4;
    int blownUpSize = HOUSE_D * blowUpFactor;

    int numBlowupPixels = blownUpSize * blownUpSize;
    
    // opt:  no need to operate on all four channels
    // just process alpha channel now

    // opt:  do all this processing with uchars instead of doubles
    unsigned char *fullGridChannelsBlownUpAlpha =
        new unsigned char[ numBlowupPixels ];

    memset( fullGridChannelsBlownUpAlpha, 0, numBlowupPixels );
    

    for( int y=0; y<HOUSE_D; y++ ) {
        for( int x=0; x<HOUSE_D; x++ ) {
            
            char hit = ( mHouseSubMapIDs[ y * HOUSE_D + x ] != 0 );
            

            if( hit ) {
                
                for( int by = y * blowUpFactor; 
                     by < (y+1) * blowUpFactor; by++ ) {
                    for( int bx = x * blowUpFactor; 
                         bx < (x+1) * blowUpFactor; bx++ ) {
                        
                        fullGridChannelsBlownUpAlpha[ by * blownUpSize + bx ]
                            = 255;
                        }
                    }
                }
            }
        }
    
    int numTouchedPixels = ( blownUpSize - 2 ) * ( blownUpSize - 2 );
    
    int *touchedPixels = new int[ numTouchedPixels ];
    
    int i = 0;
    for( int y=0; y<blownUpSize; y++ ) {
        for( int x=0; x<blownUpSize; x++ ) {
    
            if( y > 1 && y < blownUpSize - 1
                &&
                x > 1 && x < blownUpSize - 1 ) {
                
                // apply blur to all but edge pixels
                touchedPixels[ i ] = y * blownUpSize + x;
                i++;
                }
            else {
                // set edges to black
                fullGridChannelsBlownUpAlpha[ y * blownUpSize + x ] = 255;
                }
            }
        }

    FastBoxBlurFilter blur;
    
    blur.applySubRegion( fullGridChannelsBlownUpAlpha,
                         touchedPixels,
                         numTouchedPixels,
                         blownUpSize, blownUpSize );
    
    
    mWallShadowSprite = fillSpriteAlphaOnly( fullGridChannelsBlownUpAlpha,
                                             blownUpSize, 
                                             blownUpSize );

    delete [] fullGridChannelsBlownUpAlpha;
    */
    }




void HouseGridDisplay::logEdit( int inFullIndex, int inNewID ) {
    GridChangeRecord r;
    
    r.fullIndex = inFullIndex;
    r.oldID = mHouseMapIDs[ inFullIndex ];
    r.oldState = mHouseMapCellStates[ inFullIndex ];

    r.oldMobileID = mHouseMapMobileIDs[ inFullIndex ];
    r.oldMobileState = mHouseMapMobileCellStates[ inFullIndex ];

    r.newID = inNewID;
    r.robberIndex = mRobberIndex;
    
    r.subMapOffsetX = mSubMapOffsetX;
    r.subMapOffsetY = mSubMapOffsetY;

    mEditHistory.push_back( r );
    }



char HouseGridDisplay::canUndo() {
    return ( mEditHistory.size() > 0 );
    }

        
// returns cost of change that was undone
int HouseGridDisplay::undo() {
    int numSteps = mEditHistory.size();
    
    if( numSteps == 0 ) {
        return 0;
        }
    

    GridChangeRecord *r = mEditHistory.getElement( numSteps - 1 );
    
    
    mHouseMapIDs[ r->fullIndex ] = r->oldID;
    mHouseMapCellStates[ r->fullIndex ] = r->oldState;

    mHouseMapMobileIDs[ r->fullIndex ] = r->oldMobileID;
    mHouseMapMobileCellStates[ r->fullIndex ] = r->oldMobileState;
    

    

    // reset to 0 state (owner presentation) for all
    // non-stuck objects
    // NOT just object that was touched by Undo
    
    // as things are changing around during edits, all bets are off after
    // an undo, and it's best to revert everything back to state-0 to ensure
    // consistency
    resetToggledStatesInternal( 0 );
    
    mLastRobberIndex = mRobberIndex;
    mRobberIndex = r->robberIndex;
    
    
    if( isPropertySet( r->newID, 0, mandatory ) ) {
        // this change placed a mandatory object
        // undoing it means that mandatory still needs placing
        mMandatoryNeedsPlacing = true;
        mMandatoryToPlaceID = r->newID;
        }
    else if( isPropertySet( r->oldID, 0, mandatory ) ) {
        // rolling back to a placed mandatory
        mMandatoryNeedsPlacing = false;
        if( r->oldID == GOAL_ID ) {
            mGoalIndex = r->fullIndex;
            }
        }
    
    // force copy-back and shadow recompute, plus possible view move
    setVisibleOffset( r->subMapOffsetX, r->subMapOffsetY );
    

    int cost = mPicker->getPrice( r->newID );
    
    if( cost == -1 ) {
        cost = 0;
        }
    
    

    mEditHistory.deleteElement( numSteps - 1 );


    checkExitPaths();
    
    return cost;
    }




void HouseGridDisplay::setTargetHighlights( 
    SimpleVector<int> *inToolTargetFullIndices ) {

    mToolTargetFullIndices.deleteAll();
    
    mToolTargetPickedFullIndex = -1;

    int *array = inToolTargetFullIndices->getElementArray();
    int size = inToolTargetFullIndices->size();
    
    mToolTargetFullIndices.push_back( array, size );
    
    delete [] array;
    }



void HouseGridDisplay::setPickedTargetHighlight( int inPickedFullIndex ) {
    mToolTargetPickedFullIndex = inPickedFullIndex;
    }



void HouseGridDisplay::addSpecialHighlight( int inFullIndex, 
                                            SpriteHandle inSprite ) {
    
    mSpecialHighlightFullIndices.push_back( inFullIndex );
    mSpecialHighlightSprites.push_back( inSprite );
    }



void HouseGridDisplay::clearSpecialHighlights() {
    mSpecialHighlightFullIndices.deleteAll();
    mSpecialHighlightSprites.deleteAll();
    }





char *HouseGridDisplay::getBlockedMap() {
    char *blockedMap = new char[mNumMapSpots];
    for( int i=0; i<mNumMapSpots; i++ ) {
        if( mHouseMapIDs[i] != 0 &&
            !isPropertySet( mHouseMapIDs[i], mHouseMapCellStates[i],
                            family ) &&
            !isPropertySet( mHouseMapIDs[i], mHouseMapCellStates[i],
                            mobile ) ) {
            // not mobile, not family, not bare floor
            blockedMap[i] = true;
            }
        else {
            blockedMap[i] = false;
            }
        
        if( ! mDeadFamilyBlocking && 
            isPropertySet( mHouseMapIDs[i], mHouseMapCellStates[i],
                           deadFamily ) ) {
            blockedMap[i] = false;
            }
        }
    return blockedMap;
    }



void HouseGridDisplay::checkExitPaths() {
    
    char *blockedMap = getBlockedMap();
    
    GridPos goalPos = { mStartIndex % mFullMapD, mStartIndex / mFullMapD };
    

    mAllFamilyObjectsHaveExitPath = true;
    
    for( int i=0; i<mFamilyExitPaths.size(); i++ ) {
        delete [] *( mFamilyExitPaths.getElement( i ) );
        }
    mFamilyExitPaths.deleteAll();
    mFamilyExitPathLengths.deleteAll();
    
    
    for( int i=0; i<mNumMapSpots; i++ ) {
        if( isPropertySet( mHouseMapIDs[i], 
                           mHouseMapCellStates[i],
                           family ) ) {
            
            GridPos startPos = { i % mFullMapD, i / mFullMapD };
            
            int fullPathLength;
            GridPos *fullPath;
            
            char found = pathFind( mFullMapD, mFullMapD,
                                   blockedMap, 
                                   startPos, goalPos, 
                                   &fullPathLength,
                                   &fullPath );
            
            if( found && fullPathLength != 0 ) {
                mFamilyExitPaths.push_back( fullPath );
                mFamilyExitPathLengths.push_back( fullPathLength );
                }
            else {
                fullPath = new GridPos[1];
                fullPath[0] = startPos;

                mFamilyExitPaths.push_back( fullPath );
                mFamilyExitPathLengths.push_back( 1 );
                
                mAllFamilyObjectsHaveExitPath = false;
                }
            }
        }
    
    delete [] blockedMap;
    }



int HouseGridDisplay::posToIndex( GridPos inPos ) {
    return mFullMapD * inPos.y + inPos.x;
    }

    




static int nextShotNumber = -1;
static char shotDirExists = false;



void HouseGridDisplay::saveWholeMapImage() {

    File shotDir( NULL, "mapShots" );
    
    if( !shotDirExists && !shotDir.exists() ) {
        shotDir.makeDirectory();
        shotDirExists = shotDir.exists();
        }
    
    if( nextShotNumber < 1 ) {
        if( shotDir.exists() && shotDir.isDirectory() ) {
        
            int numFiles;
            File **childFiles = shotDir.getChildFiles( &numFiles );

            nextShotNumber = 1;

            char *formatString = autoSprintf( "map%%d.tga" );

            for( int i=0; i<numFiles; i++ ) {
            
                char *name = childFiles[i]->getFileName();
                
                int number;
                
                int numRead = sscanf( name, formatString, &number );
                
                if( numRead == 1 ) {
                    
                    if( number >= nextShotNumber ) {
                        nextShotNumber = number + 1;
                        }
                    }
                delete [] name;
                
                delete childFiles[i];
                }
            
            delete [] formatString;
            
            delete [] childFiles;
            }
        }
    

    if( nextShotNumber < 1 ) {
        return;
        }
    
    char *fileName = autoSprintf( "map%05d.tga", nextShotNumber );
    
    nextShotNumber++;
    

    File *file = shotDir.getChildFile( fileName );
    
    delete [] fileName;



    fileName = file->getFullFileName();
    

    delete file;


    

    int oldXOffset = getVisibleOffsetX();
    int oldYOffset = getVisibleOffsetY();
    int oldHighlightIndex = mHighlightIndex;
    
    // temporarily disable highlight during map image
    mHighlightIndex = -1;

    int fullMapD = getFullMapD();
    
    // for speed, split into blocks of tiles and grab images of entire block
    int blockD = 1;
    
    // find largest possible block size that both
    // A) fits on screen
    // B) evenly divides the whole map size
    for( int i=2; i<=HOUSE_D; i++ ) {
        
        int candidateNumBlocks = fullMapD / i;
        
        if( candidateNumBlocks * i == fullMapD ) {
            // evenly divides whole map
            
            // largest even divisor found so far
            blockD = i;
            }
        }


    int numBlockD = fullMapD / blockD;

    Image **blockImages = new Image*[ numBlockD * numBlockD ];
    
    for( int yb=0; yb<numBlockD; yb++ ) {
        // y of first tile in block
        int y = yb * blockD;
        
        // buffer away from edge if possible to avoid edge shadows
        int viewOffsetY = y - 1;
        
        if( viewOffsetY < 0 ) {
            viewOffsetY = 0;
            }
        else if( viewOffsetY > fullMapD - HOUSE_D ) {
            viewOffsetY = fullMapD - HOUSE_D;
            }
        
        for( int xb=0; xb<numBlockD; xb++ ) {
            // x of first tile in block
            int x = xb * blockD;

            int viewOffsetX = x - 1;
        
            if( viewOffsetX < 0 ) {
                viewOffsetX = 0;
                }
            else if( viewOffsetX > fullMapD - HOUSE_D ) {
                viewOffsetX = fullMapD - HOUSE_D;
                }
            //printf( "Setting offset to %d,%d\n", viewOffsetX, viewOffsetY );
            setVisibleOffset( viewOffsetX, viewOffsetY );
            
            // call OUR draw function, so that even sub classes that add
            // overlays will use our pure, no-overlay draw function 
            // when they try to save the entire map
            HouseGridDisplay::draw();

            int yDistFromViewEdge = y - viewOffsetY;
            int xDistFromViewEdge = x - viewOffsetX;

            blockImages[yb * numBlockD + xb] = 
                getScreenRegion( -HOUSE_D * TILE_RADIUS + 
                                 xDistFromViewEdge * TILE_RADIUS * 2,
                                 -HOUSE_D * TILE_RADIUS +
                                 yDistFromViewEdge * TILE_RADIUS * 2,
                                 2 * TILE_RADIUS * blockD,
                                 2 * TILE_RADIUS * blockD );
            }
        }
    
    
    int blockPixelD = blockImages[0]->getWidth();
    
    int fullImageD = numBlockD * blockPixelD;

    Image *wholeImage = new Image( fullImageD, fullImageD, 3, 0 );
    
    double *channels[3];
    int c;
    for( c=0; c<3; c++ ) {
        channels[c] = wholeImage->getChannel( c );
        }
    

    for( int y=0; y<numBlockD; y++ ) {
        for( int x=0; x<numBlockD; x++ ) {
            // bottom tiles are at y=0
            int blockY = (numBlockD - y) - 1;
            Image *block = blockImages[blockY * numBlockD + x];
            
            int xOffset = x * blockPixelD;
            int yOffset = y * blockPixelD;
            
            for( c=0; c<3; c++ ) {
                double *blockChannel = block->getChannel( c );
                
                for( int by=0; by<blockPixelD; by++ ) {
                    memcpy( 
                        &( channels[c][ 
                               (yOffset + by) * fullImageD + xOffset ] ), 
                        &( blockChannel[ 
                               by * blockPixelD ] ),
                        blockPixelD * sizeof( double ) );
                    }
                }
            delete blockImages[blockY * numBlockD + x];
            }
        }
    
    delete [] blockImages;
        
    writeTGAFile( fileName, wholeImage );
    
    delete [] fileName;
    delete wholeImage;    
    

    // restore position and highlight
    setVisibleOffset( oldXOffset, oldYOffset );
    mHighlightIndex = oldHighlightIndex;
    }
