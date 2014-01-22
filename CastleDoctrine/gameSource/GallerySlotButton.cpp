#include "GallerySlotButton.h"

#include "galleryObjects.h"


int GallerySlotButton::sInstanceCount = 0;

SpriteHandle GallerySlotButton::sFrameSprite = NULL;
SpriteHandle GallerySlotButton::sTagSprite = NULL;


GallerySlotButton::GallerySlotButton( Font *inDisplayFont,
                                      double inX, double inY, 
                                      double inDrawScale ) 
        : InventorySlotButton( inDisplayFont, inX, inY, inDrawScale  ) {
    
    sInstanceCount++;
    if( sFrameSprite == NULL ) {
        sFrameSprite = loadSprite( "frame.tga", true );
        }
    if( sTagSprite == NULL ) {
        sTagSprite = loadSprite( "bidMarker.tga", true );
        }
    
    // room for border around gallery object sprite (they're all 32x32)
    mWide = inDrawScale * 64;
    mHigh = inDrawScale * 64;
    }



GallerySlotButton::~GallerySlotButton() {
    sInstanceCount--;
    if( sInstanceCount == 0 ) {
        if( sFrameSprite != NULL ) {
            freeSprite( sFrameSprite );
            sFrameSprite = NULL;
            }
        if( sTagSprite != NULL ) {
            freeSprite( sTagSprite );
            sTagSprite = NULL;
            }
        
        }
    }



void GallerySlotButton::setObject( int inID ) {
    if( inID == -1 ) {
        InventorySlotButton::setObject( inID );
        setVisible( false );
        }
    else {
        mObjectID = inID;
        
        setMouseOverTip( getGalleryObjectDescription( mObjectID ) );
        mSprite = getGalleryObjectSprite( mObjectID );
        mQuantity = 1;

        setVisible( true );
        }
    }


void GallerySlotButton::drawBorder() {

    doublePair center = { 0, 0 };
        
    drawSprite( sFrameSprite, center, mDrawScale * 2 );
    }


void GallerySlotButton::draw() {
    InventorySlotButton::draw();
    

    // superclass ring covered up by frame.
    if( mRingOn ) {
        doublePair pos = { 1.125, -1.125 };
        
        
        // drop shadow first
        doublePair shadowPos = pos;
        shadowPos.x -= 0.0625;
        shadowPos.y += 0.0625;
        
        setDrawColor( 0, 0, 0, 0.25 );
        toggleLinearMagFilter( true );
        drawSprite( sTagSprite, shadowPos, mDrawScale * 2 );
        toggleLinearMagFilter( false );
        
        
        setDrawColor( 1, 1, 1, 1 );
        

        drawSprite( sTagSprite, pos, mDrawScale * 2 );
        }
    }

    

