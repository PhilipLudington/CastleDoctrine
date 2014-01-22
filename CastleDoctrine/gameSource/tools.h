#ifndef TOOLS_INCLUDED
#define TOOLS_INCLUDED


#include "minorGems/game/gameGraphics.h"



void initTools();

void freeTools();


// false for bad tool IDs
char getToolInRange( int inObjectID );


SpriteHandle getToolSprite( int inObjectID );

int getToolReach( int inObjectID );


// gets the internal name of a tool, examples:  "saw" or "water"
const char *getToolName( int inObjectID );


// gets the human-readable description, example:  "Carpentry Saw" or "Water"
const char *getToolDescription( int inObjectID  );
// examples:  "Capentry Saws" or "Bottles of Water"
const char *getToolDescriptionPlural( int inObjectID  );


// returns -1 on failure
int getToolID( const char *inName );

// gets full list of IDs that have been loaded
// result destroyed by caller
int *getFullToolIDList( int *outNumIDs );



#endif
