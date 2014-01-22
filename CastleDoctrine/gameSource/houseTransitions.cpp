// if you're updating transitions, you need to set this flag to true
// and run the game once to generate a signature for transitions.txt.
// After doing this once, you can set the flag back to false and recompile.
// (or leave it at false if you don't want the game to check signatures at
//  all).
// Please don't abuse this in clients that connect to the main server (don't
// give yourself the power to cut through concrete walls with water, for
// example).
//
// If you change transitions without regenerating signatures,
// loading will fail.
static char regenerateTransitionSignatures = false;

static const char *transitionSignatureKey = "Please don't abuse this key.";



#include "houseTransitions.h"


#include "houseObjects.h"
#include "tools.h"



#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/game/game.h"

#include "minorGems/io/file/File.h"


#include <stdlib.h>
#include <limits.h>



// set to true to make powered objects reset at the start of every step
// before applying power propagation and transitions
static char poweredObjectsAreMemoryless = false;



static char mobileObjectsFrozen = false;

void freezeMobileObjects( char inFreeze ) {
    mobileObjectsFrozen = inFreeze;
    }

char areMobilesFrozen() {
    return mobileObjectsFrozen;
    }



// describes a transition
typedef struct TransitionRecord {
        int triggerID;

        int triggerState;

        int targetID;
        int targetStartState;
        
        int targetEndState;
    } TransitionRecord;




// contains all transitions for a given trigger
typedef struct TransitionTriggerRecord {
    
        int triggerID;
        

        // can be 0 and NULL if no transitions defined for this triggerID
        int numTransitions;
        TransitionRecord *transitions;
        
        // used when parsing records
        int numFilled;
        
    } TransitionTriggerRecord;


static int numTriggerRecords = 0;
static TransitionTriggerRecord *triggerRecords = NULL;




#define NUM_BUILT_IN_TRIGGERS 9

static const char *builtInTriggerNames[NUM_BUILT_IN_TRIGGERS] = { 
    "player",
    "mobile",
    "noMobile",
    "power",
    "noPower",
    "powerNorth",
    "noPowerNorth",
    "visible",
    "notVisible"
    };


enum triggerID {
	trigger_player = 10000,
    trigger_mobile,
    trigger_noMobile,
    trigger_power,
    trigger_noPower,
    trigger_powerNorth,
    trigger_noPowerNorth,
    trigger_visible,
    trigger_notVisible,
    trigger_END
    };                                              
    

// built-in trigger IDs start at 10000
// other trigger IDs match objectIDs
static int getTriggerID( char *inTriggerName ) {
    for( int i=0; i<NUM_BUILT_IN_TRIGGERS; i++ ) {
        
        if( strcmp( builtInTriggerNames[i], inTriggerName ) == 0 ) {
            return i + 10000;
            }
        }

    int toolID = getToolID( inTriggerName );
    
    if( toolID != -1 ) {
        return toolID;
        }
    else {
        return getObjectID( inTriggerName );
        }
    }



#include "minorGems/crypto/hashes/sha1.h"

// sha1 digest of reach ascii base-10 numbers in each transition concatonated
// together in the order in which the transitions were parsed, concatonated
// with the key from above
static char *computeTransitionSignature( 
    SimpleVector<TransitionRecord> *inRecords ) {
    
    SimpleVector<char> stringBuffer;
    
    for( int i=0; i<inRecords->size(); i++ ) {
        TransitionRecord r = *( inRecords->getElement( i ) );
        
        char *rString = autoSprintf( "%d %d %d %d %d ",
                                     r.triggerID,
                                     r.triggerState,
                                     r.targetID,
                                     r.targetStartState,
                                     r.targetEndState );
        stringBuffer.appendElementString( rString );
        delete [] rString;
        }
    stringBuffer.appendElementString( transitionSignatureKey );

    char *transitionString = stringBuffer.getElementString();
    
    char *sig = computeSHA1Digest( transitionString );

    delete [] transitionString;
    
    return sig;
    }




void initHouseTransitions() {
    File elementsDir( NULL, "gameElements" );

    if( !elementsDir.exists() || !elementsDir.isDirectory() ) {
        return;
        }
    
    File *transitionsFile = elementsDir.getChildFile( "transitions.txt" );
    
    if( transitionsFile == NULL ) {
        return;
        }
    else if( !transitionsFile->exists() || transitionsFile->isDirectory() ) {
        delete transitionsFile;
        return;
        }

    char *transitionsText = transitionsFile->readFileContents();
    delete transitionsFile;


    // turn all newlines into \n
    
    char found;
    char *cleanedText1 = replaceAll( transitionsText, "\r\n", "\n", &found );
    
    delete [] transitionsText;
    
    char *cleanedText2 = replaceAll( cleanedText1, "\r", "\n", &found );

    delete [] cleanedText1;
    
    
    // process lines

    int numLines;
    char **lines = split( cleanedText2, "\n", &numLines );
    
    delete [] cleanedText2;
    

    SimpleVector<TransitionRecord> parsedRecords;

    for( int i=0; i<numLines; i++ ) {
        char *trimmedLine = trimWhitespace( lines[i] );
        delete [] lines[i];

        if( strcmp( trimmedLine, "" ) != 0 ) {
            
            // samples:
            // player   #   floor_trap:1   =>  2
            // player   #   floor_trap   =>  2
            // floor_trap:2   #   pitbull:1   =>  2


            
            char triggerName[100];
            char targetName[100];
            
            int triggerState;

            int startState;
            int endState;
            
            
            // :state parts are optional
            // count them
            int colonCount = 0;
            int lineLength = strlen( trimmedLine );
            
            for( int c=0; c<lineLength; c++ ) {
                if( trimmedLine[c] == ':' ) {
                    colonCount ++;
                    }
                }
            
            char scanFail = false;

            if( colonCount == 0 ) {
                int numRead = sscanf( 
                    trimmedLine, 
                    "%99[^# \t]   #   %99[^=> \t]   =>   %d", 
                    triggerName, targetName, 
                    &endState );
                
                if( numRead != 3 ) {
                    scanFail = true;
                    }
                else {
                    triggerState = -1;
                    startState = -1;
                    }
                }
            else if( colonCount == 2 ) {
                int numRead = sscanf( 
                    trimmedLine, 
                    "%99[^:# \t] : %d   #   %99[^:=> \t] : %d   =>   %d", 
                    triggerName, &triggerState, targetName, 
                    &startState, &endState );
            

                if( numRead != 5 ) {
                    scanFail = true;
                    }                
                }
            else if( colonCount == 1 ) {
                // a :state part for either the trigger or the target
                
                int numRead = sscanf( 
                    trimmedLine, 
                    "%99[^:# \t] : %d   #   %99[^=> \t]  =>   %d", 
                    triggerName, &triggerState, targetName, &endState );

                if( numRead != 4 ) {
                    // try opposite
                    numRead = sscanf( 
                    trimmedLine, 
                    "%99[^# \t]   #   %99[^:=> \t] : %d   =>   %d", 
                    triggerName, targetName, 
                    &startState, &endState );
                    
                    if( numRead != 4 ) {
                        scanFail = true;
                        }
                    else {
                        triggerState = -1;
                        }                    
                    }
                else {
                    startState = -1;
                    }
                }
            else {
                scanFail = true;
                }


            if( ! scanFail ) {
                
                TransitionRecord r;
            
                r.triggerID = getTriggerID( triggerName );
                
                r.triggerState = triggerState;

                r.targetID = getObjectID( targetName );
                
                r.targetStartState = startState;
                r.targetEndState = endState;
                
                if( r.triggerID != -1 && r.targetID != -1 ) {
                    parsedRecords.push_back( r );
                    }
                else {
                    scanFail = true;
                    }
                }

            if( scanFail ) {
                printf( "Failed to parse transition line:\n"
                        "    '%s'\n", trimmedLine );
                }

            }
        
        delete [] trimmedLine;
        }
    
    delete [] lines;


    
    SimpleVector<int> uniqueTriggerIDs;
    
    SimpleVector<int> occurrenceCounts;

    // at least enough bins in array to handle all of our built-in triggers
    // Thus, even if a built-in doesn't occur in our transitions.txt file,
    // we still make a record for it with a 0-count in it.
    int maxTriggerID = trigger_END;

    for( int i=0; i<parsedRecords.size(); i++ ) {
    
        TransitionRecord *r = parsedRecords.getElement(i);

        int exisitingIndex = uniqueTriggerIDs.getElementIndex( r->triggerID );
        if( exisitingIndex == -1 ) {
            
            uniqueTriggerIDs.push_back( r->triggerID );
            occurrenceCounts.push_back( 1 );
            
            if( r->triggerID > maxTriggerID ) {
                maxTriggerID = r->triggerID;
                }
            }
        else {
            ( *( occurrenceCounts.getElement( exisitingIndex ) ) ) ++;
            }
        }


    // make trigger-indexed bins
    
    numTriggerRecords = maxTriggerID + 1;
    
    triggerRecords = new TransitionTriggerRecord[ numTriggerRecords ];
    
    for( int i=0; i<numTriggerRecords; i++ ) {
        triggerRecords[i].triggerID = i;
        triggerRecords[i].numTransitions = 0;
        triggerRecords[i].transitions = NULL;
        }
    

    for( int i=0; i<uniqueTriggerIDs.size(); i++ ) {
        
        int id = *( uniqueTriggerIDs.getElement( i ) );
        int count = *( occurrenceCounts.getElement( i ) );
        
        triggerRecords[id].numTransitions = count;
        triggerRecords[id].numFilled = 0;
        if( count != 0 ) {
            triggerRecords[id].transitions = new TransitionRecord[ count ];
            }
        else {
            triggerRecords[id].transitions = NULL;
            }
        }

    
    // finally, stick each parsed transition into an appropriate bin

    for( int i=0; i<parsedRecords.size(); i++ ) {
        
        TransitionRecord r = *( parsedRecords.getElement(i) );
        
        TransitionTriggerRecord *triggerRecord = 
            &( triggerRecords[ r.triggerID ] );
        
        triggerRecord->transitions[ triggerRecord->numFilled ] = r;
        
        triggerRecord->numFilled ++;
        }



    // sig check
    File *transitionsSigFile = 
        elementsDir.getChildFile( "transitionsSignature.txt" );
    char *transitionsSigContents = NULL;
                
    if( transitionsSigFile->exists() ) {
        transitionsSigContents = transitionsSigFile->readFileContents();    
        }       

    char transitionsSigOK = true;
    
    if( regenerateTransitionSignatures ) {
        // ignore transitionsSignature.txt and generate a new one
        char *newSig = computeTransitionSignature( &parsedRecords );
                    
        transitionsSigFile->writeToFile( newSig );
        delete [] newSig;
        }
    else if( transitionsSigContents == NULL ) {
        transitionsSigOK = false;
        }
    else {
        // else check it
        char *sig = trimWhitespace( transitionsSigContents );
        
        char *trueSig = computeTransitionSignature( &parsedRecords );
        
        if( strcmp( trueSig, sig ) != 0 ) {
            transitionsSigOK = false;
            }
        delete [] sig;
        delete [] trueSig;
        }
                
    if( transitionsSigContents != NULL ) {
        delete [] transitionsSigContents;
        }
    
    
    delete transitionsSigFile;
         
                
                
    if( !transitionsSigOK ) {
        char *dirName = elementsDir.getFullFileName();
        char *message = autoSprintf( 
            "%s\n%s",
            translate( "badTransitionSignature" ),
            dirName );
        delete [] dirName;
                    
        loadingFailed( message );
        delete [] message;
        }

    

    }



void freeHouseTransitions() {
    for( int i=0; i<numTriggerRecords; i++ ) {
        TransitionTriggerRecord *triggerRecord = &( triggerRecords[ i ] );
    
        if( triggerRecord->transitions != NULL ) {
            delete [] triggerRecord->transitions;
            }
        }
    delete [] triggerRecords;
    triggerRecords = NULL;
    numTriggerRecords = 0;    
    }



#include "minorGems/crypto/hashes/sha1.h"

// computes a digest of a map state
// resulting string destroyed by caller
static char *getMapStateChecksum( int *inMapStates, int inMapW, int inMapH ) {
    
    int numCells = inMapW * inMapH;
        
    return
        computeSHA1Digest( (unsigned char*)inMapStates, 
                           numCells * sizeof( int ) );
    
    }




// shortcut for checking mobile properties, where it returns false
// if mobile not present (inID = 0) instead of querying the property database.
inline static char isMobilePropertySet( int inID, int inState, 
                                        propertyID inProperty ) {
    if( inID == 0 ) {
        return false;
        }
    else {
        return isPropertySet( inID, inState, inProperty );
        }
    }
    


// returns power map with (inMapW * inMapH) cells
static char *propagatePower(  int *inMapIDs, 
                              int *inMapStates,
                              int *inMapMobileIDs, int *inMapMobileStates,
                              int inMapW, int inMapH,
                              char **outTopBottomPowerMap ) {
    
    int numCells = inMapW * inMapH;

    char *powerMap = new char[ numCells ];
    memset( powerMap, false, numCells );

    // separately track whether each cell is passing power only left-to-right
    // or only top-to-bottom (can be both!)
    // fully conductive cells that become powered have both of these set
    // 
    // cells that are powered but only conductiveInternal have NEITHER
    // of these set (don't pass power at all) and also have flag
    // set in internalPowerMap below
    char *leftRightPowerMap = new char[numCells];
    memset( leftRightPowerMap, false, numCells );

    char *topBottomPowerMap = new char[numCells];
    memset( topBottomPowerMap, false, numCells );


    char *internalPowerMap = new char[numCells];
    memset( internalPowerMap, false, numCells );


    // cache properties so that we don't have to ask for them over and
    // over in loop below
    char *conductiveCells = new char[numCells];
    memset( conductiveCells, false, numCells );
    
    char *conductiveLeftRightCells = new char[numCells];
    memset( conductiveLeftRightCells, false, numCells );
    
    char *conductiveTopBottomCells = new char[numCells];
    memset( conductiveTopBottomCells, false, numCells );

    char *conductiveInternalCells = new char[numCells];
    memset( conductiveInternalCells, false, numCells );

    
    char *notAtAllConductive = new char[numCells];
    memset( notAtAllConductive, true, numCells );


    char change = false;
    
    // first, start power at cells that are powered currently

    for( int i=0; i<numCells; i++ ) {
        if( isPropertySet( inMapIDs[i], inMapStates[i], powered ) 
            ||
            isMobilePropertySet( inMapMobileIDs[i], inMapMobileStates[i], 
                             powered ) ) {
            powerMap[i] = true;
            leftRightPowerMap[i] = true;
            topBottomPowerMap[i] = true;
            change = true;
            }
        }

    // cache conductive properties, but only if they matter
    if( change ) {
        for( int i=0; i<numCells; i++ ) {
            if( isPropertySet( inMapIDs[i], inMapStates[i], conductive ) 
                ||
                isMobilePropertySet( inMapMobileIDs[i], inMapMobileStates[i], 
                                     conductive ) ) {
                conductiveCells[i] = true;
                notAtAllConductive[i] = false;
                }
            else {
                if( isPropertySet( inMapIDs[i], inMapStates[i], 
                                   conductiveLeftToRight ) 
                    ||
                    isMobilePropertySet( inMapMobileIDs[i], 
                                         inMapMobileStates[i], 
                                         conductiveLeftToRight ) ) {
                    conductiveLeftRightCells[i] = true;
                    notAtAllConductive[i] = false;
                    }
                
                if( isPropertySet( inMapIDs[i], inMapStates[i], 
                                   conductiveTopToBottom ) 
                    ||
                    isMobilePropertySet( inMapMobileIDs[i], 
                                         inMapMobileStates[i], 
                                         conductiveTopToBottom ) ) {
                    conductiveTopBottomCells[i] = true;
                    notAtAllConductive[i] = false;
                    }

                if( isPropertySet( inMapIDs[i], inMapStates[i], 
                                   conductiveInternal ) 
                    ||
                    isMobilePropertySet( inMapMobileIDs[i], 
                                         inMapMobileStates[i], 
                                         conductiveInternal ) ) {
                    conductiveInternalCells[i] = true;
                    notAtAllConductive[i] = false;
                    }
                }
            }    
        }


    // optimization:
    // actually conductive cells, that participate in the loop below,
    // are often sparse.
    // extract them now so we can loop over fewer cells during propagation
    
    SimpleVector<int> conductiveIndicesList;
    
    for( int i=0; i<numCells; i++ ) {
        if( ! notAtAllConductive[i] ) {
            conductiveIndicesList.push_back( i );
            }
        }

    int numConductive = conductiveIndicesList.size();
    int *conductiveIndices = conductiveIndicesList.getElementArray();
    


    while( change ) {
        // keep propagating power through conductive materials
        change = false;
        
        for( int c=0; c<numConductive; c++ ) {
            int i = conductiveIndices[c];

            if( powerMap[i] &&
                leftRightPowerMap[i] &&
                topBottomPowerMap[i] ) {
                // ignore cells already fully powered
                continue;
                }
            

            if( conductiveCells[i] ) {
                // look for neighbors with power
                // continue as soon as one found
                
                int y = i / inMapW;
                int x = i % inMapW;
                
                if( y > 0 &&
                    powerMap[ i - inMapW ] &&
                    ! internalPowerMap[ i - inMapW ] &&
                    ( ! leftRightPowerMap[ i - inMapW ]
                      || topBottomPowerMap[ i - inMapW ] ) ) {
                    powerMap[i] = true;
                    leftRightPowerMap[i] = true;
                    topBottomPowerMap[i] = true;
                    change = true;
                    continue;
                    }
                if( y < inMapH - 1 &&
                    powerMap[ i + inMapW ] &&
                    ! internalPowerMap[ i + inMapW ] &&
                    ( ! leftRightPowerMap[ i + inMapW ]
                      || topBottomPowerMap[ i + inMapW ] ) ) {
                    powerMap[i] = true;
                    leftRightPowerMap[i] = true;
                    topBottomPowerMap[i] = true;
                    change = true;
                    continue;
                    }
                if( x > 0 &&
                    powerMap[ i - 1 ] &&
                    ! internalPowerMap[ i - 1 ] &&
                    ( leftRightPowerMap[ i - 1 ]
                      || ! topBottomPowerMap[ i - 1 ] ) ) {
                    powerMap[i] = true;
                    leftRightPowerMap[i] = true;
                    topBottomPowerMap[i] = true;
                    change = true;
                    continue;
                    }            
                if( x < inMapW - 1 &&
                    powerMap[ i + 1 ] &&
                    ! internalPowerMap[ i + 1 ] &&
                    ( leftRightPowerMap[ i + 1 ]
                      || ! topBottomPowerMap[ i + 1 ] ) ) {
                    powerMap[i] = true;
                    leftRightPowerMap[i] = true;
                    topBottomPowerMap[i] = true;
                    change = true;
                    continue;
                    }
                }
            else {
                if( !leftRightPowerMap[i] && conductiveLeftRightCells[i] ) {
                    
                    // look for left or right neighbor with power
                    int x = i % inMapW;
                    
                    if( x > 0 &&
                        powerMap[ i - 1 ] &&
                        ! internalPowerMap[ i - 1 ] &&
                        ( leftRightPowerMap[ i - 1 ]
                          || ! topBottomPowerMap[ i - 1 ] )  ) {
                        
                        powerMap[i] = true;
                        leftRightPowerMap[i] = true;
                        change = true;
                        }
                    if( x < inMapW - 1 &&
                        powerMap[ i + 1 ] &&
                        ! internalPowerMap[ i + 1 ] &&
                        ( leftRightPowerMap[ i + 1 ]
                          || ! topBottomPowerMap[ i + 1 ] ) ) {
                        
                        powerMap[i] = true;
                        leftRightPowerMap[i] = true;
                        change = true;
                        }
                    }
                

                if( !topBottomPowerMap[i] && conductiveTopBottomCells[i] ) {
                    
                    // look for top or bottom neighbor with power
                    int y = i / inMapW;
                    
                    if( y > 0 &&
                        powerMap[ i - inMapW ] &&
                        ! internalPowerMap[ i - inMapW ] &&
                        ( ! leftRightPowerMap[ i - inMapW ]
                          || topBottomPowerMap[ i - inMapW ] ) ) {
                        
                        powerMap[i] = true;
                        topBottomPowerMap[i] = true;
                        change = true;
                        }
                    if( y < inMapH - 1 &&
                        powerMap[ i + inMapW ] &&
                        ! internalPowerMap[ i + inMapW ] &&
                        ( ! leftRightPowerMap[ i + inMapW ]
                          || topBottomPowerMap[ i + inMapW ] ) ) {
                        
                        powerMap[i] = true;
                        topBottomPowerMap[i] = true;
                        change = true;
                        }
                    }
                


                if( !powerMap[i] &&
                    !internalPowerMap[i] &&
                    conductiveInternalCells[i] ) {
                                
                    // look for any neighbor with power
                    int x = i % inMapW;
                    
                    if( x > 0 &&
                        powerMap[ i - 1 ] &&
                        ! internalPowerMap[ i - 1 ] &&
                        ( leftRightPowerMap[ i - 1 ]
                          || ! topBottomPowerMap[ i - 1 ] )  ) {
                        
                        powerMap[i] = true;
                        internalPowerMap[i] = true;
                        change = true;
                        }
                    if( x < inMapW - 1 &&
                        powerMap[ i + 1 ] &&
                        ! internalPowerMap[ i + 1 ] &&
                        ( leftRightPowerMap[ i + 1 ]
                          || ! topBottomPowerMap[ i + 1 ] ) ) {
                        
                        powerMap[i] = true;
                        internalPowerMap[i] = true;
                        change = true;
                        }
                    
                    int y = i / inMapW;
                    
                    if( y > 0 &&
                        powerMap[ i - inMapW ] &&
                        ! internalPowerMap[ i - inMapW ] &&
                        ( ! leftRightPowerMap[ i - inMapW ]
                          || topBottomPowerMap[ i - inMapW ] ) ) {
                        
                        powerMap[i] = true;
                        internalPowerMap[i] = true;
                        change = true;
                        }
                    if( y < inMapH - 1 &&
                        powerMap[ i + inMapW ] &&
                        ! internalPowerMap[ i + inMapW ] &&
                        ( ! leftRightPowerMap[ i + inMapW ]
                          || topBottomPowerMap[ i + inMapW ] ) ) {
                        
                        powerMap[i] = true;
                        internalPowerMap[i] = true;
                        change = true;
                        }
                    }


                }
            }
        }

    delete [] leftRightPowerMap;
    delete [] internalPowerMap;

    delete [] conductiveCells;
    delete [] conductiveLeftRightCells;
    delete [] conductiveTopBottomCells;
    delete [] conductiveInternalCells;
    delete [] notAtAllConductive;
    
    delete [] conductiveIndices;
    
    *outTopBottomPowerMap = topBottomPowerMap;

    return powerMap;
    }



// returns true if something changed, which might require an additional
// transition step for propagation
//
// Mobiles can conduct electricity, but are not affected by power
// (this allows us to implement a panic button that can be crossed and 
//  triggered by family members).
//
// Also, mobiles in the "powered" state generate power for neighboring tiles.
//
// Thus, inMapMobileIDs and inMapMobileStates are NOT changed by this call
//
// if inInit is set to true, then the state of the map is initialize
// from a "no power anywhere" initializing power condition
static char applyPowerTransitions( int *inMapIDs, 
                                   int *inMapStates,
                                   int *inMapMobileIDs, int *inMapMobileStates,
                                   int inMapW, int inMapH,
                                   char inInit = false ) {
    
    
    char *topBottomPowerMap;
    char *powerMap;


    if( inInit ) {
        // start from a state where nothing has power
        int numCells = inMapW * inMapH;

        powerMap = new char[ numCells ];
        memset( powerMap, false, numCells );

        topBottomPowerMap = new char[ numCells ];
        memset( topBottomPowerMap, false, numCells );
        }
    else {
        // don't reset states to no-powered condition
        // continue propagating power given current states
        
        powerMap = propagatePower( inMapIDs, inMapStates,
                                   inMapMobileIDs, inMapMobileStates, 
                                   inMapW, inMapH,
                                   &topBottomPowerMap );
        }
    

    int numCells = inMapW * inMapH;

    
    // now execute transitions for cells based on power or noPower
    
    char transitionHappened = false;
    

    for( int i=0; i<numCells; i++ ) {
        
        TransitionTriggerRecord *r;
        
        SimpleVector<TransitionTriggerRecord *> applicableTriggers;
        

        if( powerMap[i]  ) {
            // this cell is powered
            r = 
                &( triggerRecords[ trigger_power ] );
            
            applicableTriggers.push_back( r );
            }
        else {            
            // this cell itself isn't powered
            r = 
                &( triggerRecords[ trigger_noPower ] );
            
            applicableTriggers.push_back( r );
            }
        
            
        // check if it's northern neighbor is powered
        int y = i / inMapW;

        char powerNorth = false;
            
        if( y < inMapH - 1 ) {
            // check above
            
            if( topBottomPowerMap[ i + inMapW ] ) {
                powerNorth = true;
                }
            }
        
        if( powerNorth ) {
            r = 
                &( triggerRecords[ trigger_powerNorth ] );
            applicableTriggers.push_back( r );
            }
        else {
            r = 
                &( triggerRecords[ trigger_noPowerNorth ] );
            
            applicableTriggers.push_back( r );
            }
        
        
        int numTriggers = applicableTriggers.size();
        
        for( int a=0; a<numTriggers; a++ ) {
            TransitionTriggerRecord *r = 
                *( applicableTriggers.getElement( a ) );
        
            for( int j=0; j<r->numTransitions; j++ ) {
        
                TransitionRecord *transRecord = 
                    &( r->transitions[j] );
            
                if( transRecord->targetID == inMapIDs[i]
                    &&
                    ( transRecord->targetStartState == inMapStates[i]
                      || 
                      transRecord->targetStartState == -1 )
                    &&
                    transRecord->targetEndState != inMapStates[i] ) {
                
                    inMapStates[ i ] = transRecord->targetEndState;

                    transitionHappened = true;
                    }
                }
            }
        
        }


    delete [] powerMap;
    delete [] topBottomPowerMap;
    
    return transitionHappened;
    }



static double distance( int inXA, int inYA, int inXB, int inYB ) {
    return sqrt( (inXA - inXB) * (inXA - inXB) +
                 (inYA - inYB) * (inYA - inYB) );
    }



static char distCompare( double inDistA, double inDistB, char inSeek ) {
    if( inSeek ) {
        return inDistA < inDistB;
        }
    else {
        return inDistA > inDistB;
        }
    }





static void applyMobileTransitions( int *inMapIDs, int *inMapStates, 
                                    int *inMapMobileIDs, 
                                    int *inMapMobileStates,
                                    int *inMapMobileStartingPositions,
                                    float *inMapMobileCellFades,
                                    int inMapW, int inMapH,
                                    int inRobberIndex,
                                    int inLastRobberIndex,
                                    int inStartIndex ) {
    
    
    // process transitions for house objects under player
    // e.g., if player opens a player-only-door, the door opens before
    // mobiles move.
    if( inMapIDs[ inRobberIndex ] != 0 ) {
        TransitionTriggerRecord *r = 
            &( triggerRecords[ trigger_player ] );
        
        int playerOnID = inMapIDs[ inRobberIndex ];
        int playerOnState = inMapStates[ inRobberIndex ];

        for( int i=0; i<r->numTransitions; i++ ) {
            TransitionRecord *transRecord = &( r->transitions[i] );
        
            if( transRecord->targetID == playerOnID
                &&
                ( transRecord->targetStartState == playerOnState
                  ||
                  transRecord->targetStartState == -1 )
                &&
                transRecord->targetEndState != playerOnState ) {
                
                inMapStates[ inRobberIndex ] = 
                    transRecord->targetEndState;
                
                // only allow one transition triggered for
                // the object that overlaps with player
                break;
                }
            }
        }




    // next, move mobile objects around

    
    // process playerSeeking properties

    int numCells = inMapW * inMapH;    

    int robberX = inRobberIndex % inMapW;
    int robberY = inRobberIndex / inMapW;

    int lastRobberX = inLastRobberIndex % inMapW;
    int lastRobberY = inLastRobberIndex / inMapW;


    char *moveHappened = new char[ numCells ];
    

    memset( moveHappened, false, numCells );


    
    // make a map of dead mobile objects
    // mobiles avoid moving there if they can help it (unless player-seeking
    // and player is standing there)
    char *deadMobilePresentAdjacent = new char[ numCells ];
    
    memset( deadMobilePresentAdjacent, false, numCells );
    
    for( int i=0; i<numCells; i++ ) {
        // for now, mobiles only become stuck when dead
        // This may change later, in which case a new property to
        // distinguish dead mobiles (like 'deadMobile' may need to be added).
        if( isPropertySet( inMapMobileIDs[i], 
                           inMapMobileStates[i], 
                           stuck ) ) {
            
            deadMobilePresentAdjacent[i] = true;
            
            // spread to neighbor cells too    
            int x = i % inMapW;
            int y = i / inMapW;
                
            if( x > 0 ) {
                deadMobilePresentAdjacent[ i - 1 ] = true;
                }
            if( x < inMapW - 1 ) {
                deadMobilePresentAdjacent[ i + 1 ] = true;
                }
            if( y > 0 ) {
                deadMobilePresentAdjacent[ i - inMapW ] = true;
                }
            if( y < inMapH - 1 ) {
                deadMobilePresentAdjacent[ i + inMapW ] = true;
                }
            }
        
        }
    
    
    
    

    for( int i=0; i<numCells; i++ ) {
        if( !moveHappened[i] &&
            ( isPropertySet( inMapMobileIDs[i], 
                             inMapMobileStates[i], playerSeeking )
              ||
              isPropertySet( inMapMobileIDs[i], 
                             inMapMobileStates[i], playerAvoiding ) ) ) {
            
            // seek or avoid?
            char seek = isPropertySet( inMapMobileIDs[i], 
                                       inMapMobileStates[i], playerSeeking );

            int x = i % inMapW;
            int y = i / inMapW;
            
            int dX = robberX - x;
            int dY = robberY - y;
            
            int destX = x;
            int destY = y;
            

            if( dX == 0 && dY == 0 ) {
                if( seek ) {
                    // no move, already on top of player
                    continue;
                    }
                else {
                    // avoiding player AND on top of player
                    // move to spot furthest from robber's last position
                    // (this feels like sensible "run away" behavior)

                    double bestDistance = distance( x, y, 
                                                    robberX, robberY );
                    double bestDistanceFromLast = 
                        distance( x, y,
                                  lastRobberX, lastRobberY );
                
                    int possibleDX[4] = { -1, 1, 0 , 0 };
                    int possibleDY[4] = { 0, 0, -1 , 1 };
                    
                    for( int d=0; d<4; d++ ) {
                        int tryX = x + possibleDX[d];
                        int tryY = y + possibleDY[d];
                        int destI = tryY * inMapW + tryX;
                        
                        double tryDist = 
                            distance( tryX, tryY, robberX, robberY );
                        double tryDistFromLast = 
                            distance( tryX, tryY, 
                                      lastRobberX, lastRobberY );
                        
                        char better = ( tryDist > bestDistance );
                        
                        if( ! better && 
                            tryDist == bestDistance ) {
                            
                            // equal... maybe better in terms of dist from
                            // last?
                            
                            if( tryDistFromLast > bestDistanceFromLast ) {
                                better = true;
                                }
                            }
                    
                        if( better && 
                            inMapMobileIDs[destI] == 0 &&
                            destI != inStartIndex &&
                            ! deadMobilePresentAdjacent[destI] &&
                            ! isPropertySet( inMapIDs[destI], 
                                             inMapStates[destI],
                                             blocking ) &&
                            ! isPropertySet( inMapIDs[destI], 
                                             inMapStates[destI],
                                             mobileBlocking ) ) {

                            destX = tryX;
                            destY = tryY;
                            bestDistance = tryDist;
                            bestDistanceFromLast = tryDistFromLast;
                            }
                        }
                    }
                }
            else {
                // seeking/avoiding player, and not on top of player

                // pick unblocked spot 
                // closest (or furthest) to (or from) player

                double bestDistance = distance( x, y, robberX, robberY );
                
                int possibleDX[4] = { -1, 1, 0 , 0 };
                int possibleDY[4] = { 0, 0, -1 , 1 };
                
                for( int d=0; d<4; d++ ) {
                    int tryX = x + possibleDX[d];
                    int tryY = y + possibleDY[d];
                    int destI = tryY * inMapW + tryX;
                        
                    double tryDist = 
                        distance( tryX, tryY, robberX, robberY );
                    
                    char blockedByDead = deadMobilePresentAdjacent[destI];
                    
                    if( seek && destI == inRobberIndex ) {
                        // ignore nearby dead creatures as we jump to
                        // square where robber is standing
                        blockedByDead = false;
                        }
                    

                    if( inMapMobileIDs[destI] == 0 &&
                        ! blockedByDead &&
                        destI != inStartIndex &&
                        ! isPropertySet( inMapIDs[destI], 
                                         inMapStates[destI],
                                         blocking ) &&
                        ! isPropertySet( inMapIDs[destI], 
                                         inMapStates[destI],
                                         mobileBlocking ) &&
                        distCompare( tryDist, bestDistance, seek ) ) {
                        

                        destX = tryX;
                        destY = tryY;
                        bestDistance = tryDist;
                        }
                    }
                }


            // move to chosen dest
            int destI = destY * inMapW + destX;
            
            
            if( destI != inStartIndex &&
                inMapMobileIDs[destI] == 0 &&
                ! isPropertySet( inMapIDs[destI], inMapStates[destI],
                                 blocking ) &&
                ! isPropertySet( inMapIDs[destI], inMapStates[destI],
                                 mobileBlocking ) ) {
                
                inMapMobileIDs[destI] = inMapMobileIDs[i];
                inMapMobileStates[destI] = inMapMobileStates[i];
                inMapMobileStartingPositions[destI] = 
                    inMapMobileStartingPositions[i];
                
                inMapMobileIDs[i] = 0;
                inMapMobileStates[i] = 0;
                inMapMobileStartingPositions[i] = -1;

                if( inMapMobileCellFades[destI] !=
                    inMapMobileCellFades[i] ) {
                    // moving to a differently-faded cell
                    // copy visibility to new cell so that mobile
                    // doesn't blink in/out of existence
                    // (will fade in/out at new location instead)
                    inMapMobileCellFades[destI] = inMapMobileCellFades[i];
                    }

                // don't keep moving it if we encounter it later in loop
                moveHappened[destI] = true;
                }
            
            }
        }
    

    delete [] deadMobilePresentAdjacent;



    // now process trigger-transitions (switch plates, etc) caused by the 
    // presensce of a mobile object

    SimpleVector<int> mobileIndices;
    
    // player is a mobile object
    // and it ALWAYS moves (player movement triggers transitions, for now)
    mobileIndices.push_back( inRobberIndex );

    for( int i=0; i<numCells; i++ ) {
        // ignore objects that are moving in synch with player, to the 
        // same destination as player (would cause double mobile triggers
        // at that destination)
        // Note that non-player mobiles never co-occupy spaces with each other
        if( i != inRobberIndex &&
            inMapMobileIDs[i] != 0 ) {
            // ignore other mobiles that didn't move just now
            // (so we don't trigger same switch over and over with a stationary
            //  mobile)
            if( moveHappened[i] ) {
                mobileIndices.push_back( i );
                }
            }
        }
    
    delete [] moveHappened;


    TransitionTriggerRecord *r = 
            &( triggerRecords[ trigger_mobile ] );

    
    for( int j=0; j<mobileIndices.size(); j++ ) {
        int mobIndex = *( mobileIndices.getElement( j ) );
        
        int mobOverTileID = inMapIDs[ mobIndex ];
        int mobOverTileState = inMapStates[ mobIndex ];

        for( int i=0; i<r->numTransitions; i++ ) {
        
            TransitionRecord *transRecord = &( r->transitions[i] );
        
            if( transRecord->targetID == mobOverTileID
                &&
                ( transRecord->targetStartState == mobOverTileState
                  ||
                  transRecord->targetStartState == -1 )
                &&
                transRecord->targetEndState != mobOverTileState ) {
                
                printf( "Mobile-triggered transition hit\n" );

                inMapStates[ mobIndex ] = transRecord->targetEndState;
                
                // only allow one transition triggered per mobile object
                break;
                }
            }
        }

    

    // handle transitions triggered by lack of mobile presense
    r = &( triggerRecords[ trigger_noMobile ] );
    
    for( int i=0; i<numCells; i++ ) {
        if( inMapMobileIDs[i] == 0 && inRobberIndex != i ) {
            for( int j=0; j<r->numTransitions; j++ ) {

                TransitionRecord *transRecord = &( r->transitions[j] );

                if( transRecord->targetID == inMapIDs[i]
                    &&
                    ( transRecord->targetStartState == inMapStates[i]
                      ||
                      transRecord->targetStartState == -1 )
                    &&
                    transRecord->targetEndState != inMapStates[i] ) {
                
                    inMapStates[i] = transRecord->targetEndState;
                    }
                }
            }        
        }
    



    // now process any transitions for mobile objects
    // that occupy same tile as player

    if( inMapMobileIDs[ inRobberIndex ] != 0 ) {
        
        r = &( triggerRecords[ trigger_player ] );


        int playerOnMobID = inMapMobileIDs[ inRobberIndex ];
        int playerOnMobState = inMapMobileStates[ inRobberIndex ];
        

        for( int i=0; i<r->numTransitions; i++ ) {
            TransitionRecord *transRecord = &( r->transitions[i] );
        
            if( transRecord->targetID == playerOnMobID
                &&
                ( transRecord->targetStartState == playerOnMobState
                  ||
                  transRecord->targetStartState == -1 )
                &&
                transRecord->targetEndState != playerOnMobState ) {
                
                inMapMobileStates[ inRobberIndex ] = 
                    transRecord->targetEndState;
                
                // only allow one transition triggered for
                // the mobile that overlaps with player
                break;
                }
            }
        }
    
    

    }
    




void applyTransitions( int *inMapIDs, int *inMapStates, 
                       int *inMapMobileIDs, int *inMapMobileStates,
                       int *inMapMobileStartingPositions,
                       float *inMapMobileCellFades,
                       int inMapW, int inMapH,
                       int inRobberIndex,
                       int inLastRobberIndex,
                       int inStartIndex ) {
    
    if( !mobileObjectsFrozen ) {
        applyMobileTransitions( inMapIDs, inMapStates,
                                inMapMobileIDs, inMapMobileStates,
                                inMapMobileStartingPositions,
                                inMapMobileCellFades,
                                inMapW, inMapH, inRobberIndex,
                                inLastRobberIndex,
                                inStartIndex );
        }
    
    int numCells = inMapW * inMapH;

    
    
    // after moving mobiles around and triggering stuff with their positions
    // need to figure out if any mobiles have become powered or conductive
    // because that will change the outcome of the power propagation below

    // still, don't apply other transitions to these mobiles now, because
    // those transitions may be different later after power has propagated
    
    // for now, just focus on changes to mobiles triggered by family members
    // here
    for( int i=0; i<numCells; i++ ) {
        if( inMapMobileIDs[i] != 0 ) {
            
            int mobID = inMapMobileIDs[ i ];
            int mobState = inMapMobileStates[ i ];
            
            int mobOverTileID = inMapIDs[ i ];
            int mobOverTileState = inMapStates[ i ];
            
            if( isPropertySet( mobOverTileID, mobOverTileState, family ) ) {
                
                inMapMobileStates[ i ] = checkTransition( mobID, mobState,
                                                          mobOverTileID,
                                                          mobOverTileState );
                }
            }
        }


    // now process power transitions


    if( poweredObjectsAreMemoryless ) {
        
        // first, clear map of any old power-triggered states from last
        // global transition
        // this brings everything back to a consistent starting state
        // and ensures that power propagation is stateless (by looking at a map
        // configuration, you can compute all tile states, regarless of what
        // happened in previous steps)
        applyPowerTransitions( inMapIDs, inMapStates, 
                               inMapMobileIDs, inMapMobileStates,
                               inMapW, inMapH,
                               // init
                               true );
        }
    

    
    char transitionHappened = true;
    char loopDetected = false;
    
    // track checksums of states seen so far so that we can avoid a loop
    SimpleVector<char *> seenStates;
    
    // add start state
    seenStates.push_back( getMapStateChecksum( inMapStates, inMapW, inMapH ) );
    

    // keep track of number of transitions applied.
    // count as looping if we go too far without seeing a loop
    // (it's possible to build a circuit that takes thousands of steps to
    //  eventually loop, and we don't want to simulate that, because it
    //  effectively freezes the game [noticed by Joshua Collins]).
    int transitionCount = 0;
    int transitionLimit = 32;
        
    int *startStates = new int[ numCells ];
        
    memcpy( startStates, inMapStates, numCells * sizeof( int ) );


    while( transitionHappened && ! loopDetected ) {
        transitionHappened = 
            applyPowerTransitions( inMapIDs, inMapStates, 
                                   inMapMobileIDs, inMapMobileStates,
                                   inMapW, inMapH );

        if( transitionHappened ) {
            transitionCount++;
            
            char *newChecksum = 
                getMapStateChecksum( inMapStates, inMapW, inMapH );
            
            for( int i=0; i<seenStates.size(); i++ ) {
                char *oldChecksum = *( seenStates.getElement( i ) );
            
                if( strcmp( oldChecksum, newChecksum ) == 0 ) {
                    loopDetected = true;
                    }
                }

            if( transitionCount > transitionLimit ) {
                loopDetected = true;
                }

            seenStates.push_back( newChecksum );
            }
        }
    
    if( loopDetected ) {
        // make sure that all looping elements "settle down" into a 
        // consistent state (otherwise, we see various flip-flops due
        // to propagation times elsewhere in the map)
        
        // in case of looping, all elements involved in the loop settle
        // down into the lowest-seen state number that they encounter 
        // during execution of the loop

        if( transitionCount > transitionLimit ) {
            // hit transition limit without seeing a real loop
            
            // return to start state before running the loop again in this
            // case, because we're going to look for lowest-seen-states
            // within this transition limit instead of a full loop
            memcpy( inMapStates, startStates, numCells * sizeof( int ) );
            }
        // otherwise, DON'T return to start state, because we use the
        // current state as a return state for loop detection (and start
        // state might not actually be part of loop, so may never be returned
        // to)

        // observer the same transition limit here as we look for the 
        // lowest seen state for each cell (in the case of
        // an incomplete loop, where we've hit the transition limit before
        // actually looping, we will find the lowest seen state before
        // the transition limit was hit).
        transitionCount = 0;
        
        
        int *lowestSeenStates = new int[ numCells ];
        
        memcpy( lowestSeenStates, inMapStates, numCells * sizeof( int ) );
        
        // run one more time until we return to this state
        char *returnToChecksum = 
            getMapStateChecksum( inMapStates, inMapW, inMapH );
        

        applyPowerTransitions( inMapIDs, inMapStates, 
                               inMapMobileIDs, inMapMobileStates,
                               inMapW, inMapH );
        char *lastChecksum = 
            getMapStateChecksum( inMapStates, inMapW, inMapH );
        
        while( strcmp( lastChecksum, returnToChecksum ) != 0 &&
               transitionCount <= transitionLimit ) {
            // a mid-loop state
            
            for( int i=0; i<numCells; i++ ) {
                
                if( inMapStates[i] < lowestSeenStates[i] ) {
                    // a lower state number seen for this cell
                    lowestSeenStates[i] = inMapStates[i];
                    }
                }
            delete [] lastChecksum;

            applyPowerTransitions( inMapIDs, inMapStates,
                                   inMapMobileIDs, inMapMobileStates,
                                   inMapW, inMapH );
            lastChecksum = 
                getMapStateChecksum( inMapStates, inMapW, inMapH );
            
            transitionCount ++;
            }

        // returned to start-of-loop state
        delete [] lastChecksum;
        delete [] returnToChecksum;

        // set all cells in map to lowest-seen states from the loop
        memcpy( inMapStates, lowestSeenStates, numCells * sizeof( int ) );
        delete [] lowestSeenStates;
        }
    

    delete [] startStates;

    for( int i=0; i<seenStates.size(); i++ ) {
        delete [] *( seenStates.getElement( i ) );
        }




    // finally, apply transitions to mobile objects based on where they
    // are standing

    // do this after power has been propagated and tile states changed based
    // on power

    for( int i=0; i<numCells; i++ ) {
        if( inMapMobileIDs[i] != 0 ) {
            
            int mobID = inMapMobileIDs[ i ];
            int mobState = inMapMobileStates[ i ];
            
            
            int mobOverTileID = inMapIDs[ i ];
            int mobOverTileState = inMapStates[ i ];
            
            
            // already triggered effects of family on mobiles above,
            // before power transitions
            if( ! isPropertySet( mobOverTileID, mobOverTileState, family ) ) {
                
                inMapMobileStates[ i ] = checkTransition( mobID, mobState,
                                                          mobOverTileID,
                                                          mobOverTileState );
                }
            
            // also, reverse:
            // mobiles can trigger specific reactions in tiles they are 
            // standing
            // on (separate from "mobile" transition for switch plates)
            // for example:  wife_shotgun (a mobile) can change state of wife
            inMapStates[ i ] = checkTransition( mobOverTileID,
                                                mobOverTileState,
                                                mobID, mobState );
            }
        }


    }




int checkTransition( int inTargetID, int inTargetState,
                     int inTriggerID, int inTriggerState ) {
    
    // all transitions triggered by this trigger
    TransitionTriggerRecord *r = &( triggerRecords[ inTriggerID ] );
        
    for( int i=0; i<r->numTransitions; i++ ) {
        
        TransitionRecord *transRecord = &( r->transitions[i] );
        
        if( ( transRecord->triggerState == inTriggerState
              ||
              transRecord->triggerState == -1
              )
            &&
            transRecord->targetID == inTargetID
            &&
            ( transRecord->targetStartState == inTargetState
              ||
              transRecord->targetStartState == -1 )
            &&
            transRecord->targetEndState != inTargetState ) {
                
            // return first one found
            return  transRecord->targetEndState;
            }
        }
    
    // default
    return inTargetState;
    }




// applies transition rule for a tool to transform inMapIDs and inMapStates
void applyToolTransition( int *inMapIDs, int *inMapStates, 
                          int *inMapMobileIDs, int *inMapMobileStates,
                          int inMapW, int inMapH,
                          int inToolID, int inToolTargetIndex ) {

    printf( "Applying transition for %d at index %d\n", inToolID,
            inToolTargetIndex );
    
    
    int tileID = inMapIDs[ inToolTargetIndex ];
    int tileState = inMapStates[ inToolTargetIndex ];
    
    int mobileID = inMapMobileIDs[ inToolTargetIndex ];
    int mobileState = inMapMobileStates[ inToolTargetIndex ];

    
    // all transitions triggered by this trigger
    TransitionTriggerRecord *r = &( triggerRecords[ inToolID ] );


    if( mobileID != 0 ) {
        
        // check mobile first
        for( int i=0; i<r->numTransitions; i++ ) {
        
            TransitionRecord *transRecord = &( r->transitions[i] );
        

            // affects mobile?
            if( transRecord->targetID == mobileID
                &&
                ( transRecord->targetStartState == mobileState
                  ||
                  transRecord->targetStartState == -1 )
                &&
                transRecord->targetEndState != mobileState ) {
                
                inMapMobileStates[ inToolTargetIndex ] = 
                    transRecord->targetEndState;
                
                // return after first one found
                return;
                }

            }
        }
    
    
    // did not affect mobile (or no mobile present)
    // now check underlying tile

    for( int i=0; i<r->numTransitions; i++ ) {
        
        TransitionRecord *transRecord = &( r->transitions[i] );
        
        // affects tile?
        if( transRecord->targetID == tileID
            &&
            ( transRecord->targetStartState == tileState
              ||
              transRecord->targetStartState == -1 )
            &&
            transRecord->targetEndState != tileState ) {
            
            inMapStates[ inToolTargetIndex ] = transRecord->targetEndState;
            
            // return after first one found
            return;
            }
        }



    }




void applyVisibilityTransitions( int *inMapIDs, int *inMapStates, 
                                 int *inMapMobileIDs, int *inMapMobileStates,
                                 int inMapW, int inMapH,
                                 char *inMapTilesVisible ) {
    

    TransitionTriggerRecord *r;
    
        

    int numCells = inMapW * inMapH;
    

    for( int c=0; c<numCells; c++ ) {
        

        if( inMapTilesVisible[c] ) {
            r = &( triggerRecords[ trigger_visible ] );
            }
        else {
            r = &( triggerRecords[ trigger_notVisible ] );
            }
        
        
        int cellID = inMapIDs[c];
        int cellState = inMapStates[c];        
            
        if( cellID != 0 ) {            
            for( int i=0; i<r->numTransitions; i++ ) {
                TransitionRecord *transRecord = &( r->transitions[i] );
                
                if( transRecord->targetID == cellID
                    &&
                    ( transRecord->targetStartState == cellState
                      ||
                      transRecord->targetStartState == -1 )
                    &&
                    transRecord->targetEndState != cellState ) {
                    
                    inMapStates[c] = 
                        transRecord->targetEndState;
                    
                    // only allow one transition triggered for
                    // the object in this spot
                    break;
                    }
                }
            }



        int mobileCellID = inMapMobileIDs[c];
        int mobileCellState = inMapMobileStates[c];

        if( mobileCellID != 0 ) {            
            for( int i=0; i<r->numTransitions; i++ ) {
                TransitionRecord *transRecord = &( r->transitions[i] );
                
                if( transRecord->targetID == mobileCellID
                    &&
                    ( transRecord->targetStartState == mobileCellState
                      ||
                      transRecord->targetStartState == -1 )
                    &&
                    transRecord->targetEndState != mobileCellState ) {
                    
                    inMapMobileStates[c] = 
                        transRecord->targetEndState;
                    
                    // only allow one transition triggered for
                    // the mobile object in this spot
                    break;
                    }
                }
            }
        }



    }
