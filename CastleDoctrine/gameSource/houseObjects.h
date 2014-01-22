#ifndef HOUSE_OBJECTS_INCLUDED
#define HOUSE_OBJECTS_INCLUDED


#include "minorGems/game/gameGraphics.h"



void initHouseObjects();


void freeHouseObjects();



// gets full list of IDs that have been loaded
// result destroyed by caller
int *getFullObjectIDList( int *outNumIDs );


// gets the internal name of an object, example:  "wall_wood"
const char *getObjectName( int inObjectID );


// gets the human-readable description, example:  "Wooden Wall"
// inWifeName is inserted into object description if object has wifeOwns 
// property set

// result destroyed by caller
char *getObjectDescription( int inObjectID,
                            int inState,
                            const char *inWifeName );


// returns -1 on failure
int getObjectID( const char *inName );




// macro trick that allows us to define this list only once and use
// it in both the enum and to create the string array.
// Ensures consistency of both lists.

// found here:  
//    http://www.gamedev.net/community/forums/topic.asp?topic_id=260159

#ifdef F
  #error Macro "F" already defined
#endif

#define PROPERTY_NAMES \
    F(permanent), \
    F(mandatory), \
    F(family), \
    F(deadFamily), \
    F(wife), \
    F(son), \
    F(daughter), \
    F(blocking), \
    F(visionBlocking), \
    F(shadowMaking), \
    F(underLayerShaded), \
    F(structural), \
    F(wall), \
    F(neverFade), \
    F(deadly), \
    F(deadlyAdjacent), \
    F(stuck), \
    F(noAutoRevert), \
    F(powered), \
    F(conductive), \
    F(conductiveLeftToRight), \
    F(conductiveTopToBottom), \
    F(conductiveInternal), \
    F(mobile), \
    F(sleeping), \
    F(forceUnderShadows), \
    F(noDropShadow), \
    F(darkHaloBehind), \
    F(mobileBlocking), \
    F(playerSeeking), \
    F(playerAvoiding), \
    F(playerFacing), \
    F(playerFacingAway), \
    F(interactingWithPlayer), \
    F(wifeOwns), \
    F(onTopOfPlayer), \
    F(thinShroud), \
    F(blockSubInfo), \
    F(signedSprite) \

                



// first, let F simply resolve to the raw name
// thus, the above list expands into the body of the enum
#define F(inName) inName

enum propertyID {
	PROPERTY_NAMES,
    endPropertyID
    };



char isPropertySet( int inObjectID, int inState, propertyID inProperty );



// tests if inOtherObjectID is in inObjectID's group
char isInGroup( int inObjectID, int inOtherObjectID );





#define MAX_ORIENTATIONS 16


int getNumOrientations( int inObjectID, int inState );

char isBehindSpritePresent( int inObjectID, int inState );
char isUnderSpritePresent( int inObjectID, int inState );


// orientations based on binary interpretation of LBRT "neighbor present"
// flags
SpriteHandle getObjectSprite( int inObjectID, int inOrientation, int inState );

SpriteHandle getObjectHaloSprite( int inObjectID, int inOrientation, 
                                  int inState );

// drawn behind mobile objects
SpriteHandle getObjectSpriteBehind( int inObjectID, int inOrientation, 
                                    int inState );
// drawn under shadow layer
SpriteHandle getObjectSpriteUnder( int inObjectID, int inOrientation, 
                                   int inState );




// utility function that other systems can use

// reads a TGA sprite and shade map, applies the shade map, and
// creates a sprite for each orientation square in the file

// inTgaPath and inShadeMapTgaPath are deleted if not NULL
// returns number of orientaitons

// inForceUnderShading shades whole sprite at darkest shading level,
// but ONLY if no shade map present
int readShadeMappedSprites( char *inTgaPath, char *inShadeMapTgaPath,
                            SpriteHandle *inSpriteOrientationArray,
                            SpriteHandle *inHaloSpriteOrientationArray = NULL,
                            char inForceUnderShading = false );


// doubles image with nearest neighbor interpolation
// destroys inImage
Image *doubleImage( Image *inImage );



#endif

