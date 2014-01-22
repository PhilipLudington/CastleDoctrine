#ifndef GALLERY_SLOT_BUTTON_INCLUDED
#define GALLERY_SLOT_BUTTON_INCLUDED


#include "InventorySlotButton.h"


#include "minorGems/game/Font.h"



class GallerySlotButton : public InventorySlotButton {
        

    public:

        GallerySlotButton( Font *inDisplayFont,
                           double inX, double inY, 
                           double inDrawScale = 1.0 );
        
        virtual ~GallerySlotButton();


        // override from InventorySlotButton to handle galleryObjects
        // instead of tools
        virtual void setObject( int inID );
        
    protected:
        
        // override to draw a frame
        virtual void drawBorder();
        
        // draw bid marker instead of ring
        virtual void draw();
        

        static int sInstanceCount;
        static SpriteHandle sFrameSprite;
        static SpriteHandle sTagSprite;
    };



#endif
