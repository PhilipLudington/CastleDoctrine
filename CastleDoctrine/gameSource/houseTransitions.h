#ifndef HOUSE_TRANSITIONS_INCLUDED
#define HOUSE_TRANSITIONS_INCLUDED



void initHouseTransitions();


void freeHouseTransitions();


// when mobile objects are frozen, they don't move during applyTransitions
// and their presence on a tile does not trigger transitions for that tile
void freezeMobileObjects( char inFreeze );

char areMobilesFrozen();


// applies transition rules to transform inMapIDs and inMapStates
void applyTransitions( int *inMapIDs, int *inMapStates, 
                       int *inMapMobileIDs, int *inMapMobileStates,
                       int *inMapMobileStartingPositions,
                       float *inMapMobileCellFades,
                       int inMapW, int inMapH,
                       int inRobberIndex,
                       int inLastRobberIndex,
                       int inStartIndex );



// returns new state for inTarget
int checkTransition( int inTargetID, int inTargetState,
                     int inTriggerID, int inTriggerState );



// applies transition rule for a tool to transform inMapIDs and inMapStates
void applyToolTransition( int *inMapIDs, int *inMapStates, 
                          int *inMapMobileIDs, int *inMapMobileStates,
                          int inMapW, int inMapH,
                          int inToolID, int inToolTargetIndex );



// applies transitions that are triggered by a map tile's visibility
void applyVisibilityTransitions( int *inMapIDs, int *inMapStates, 
                                 int *inMapMobileIDs, int *inMapMobileStates,
                                 int inMapW, int inMapH,
                                 char *inMapTilesVisible );



#endif
