#ifndef GALLERY_OBJECTS_INCLUDED
#define GALLERY_OBJECTS_INCLUDED


#include "minorGems/game/gameGraphics.h"



void initGalleryObjects();

void freeGalleryObjects();


SpriteHandle getGalleryObjectSprite( int inObjectID );


// gets the internal name of a tool, example:  "saw"
const char *getGalleryObjectName( int inObjectID );


// gets the human-readable description, example:  "Carpentry Saw"
const char *getGalleryObjectDescription( int inObjectID  );


// returns -1 on failure
int getGalleryObjectID( const char *inName );



#endif
