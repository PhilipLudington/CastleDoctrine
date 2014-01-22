#ifndef EYEDROPPER_STATUS_DISPLAY_INCLUDED
#define EYEDROPPER_STATUS_DISPLAY_INCLUDED


#include "PageComponent.h"

#include "minorGems/game/gameGraphics.h"


// Status display that explains CTRL-Click eyedropper functionality
// graphics/eyedropperOn.tga and graphics/eyedropperOff.tga
// must exist for this display to work
class EyedropperStatusDisplay : public PageComponent {
        
    public:
        
        // centered on inX, inY
        EyedropperStatusDisplay( double inX, double inY,
                                 double inDrawScale = 1.0 );
        
        ~EyedropperStatusDisplay();
        


    private:

        virtual void draw();

        virtual void pointerMove( float inX, float inY );
        virtual void pointerDown( float inX, float inY );
        virtual void pointerDrag( float inX, float inY );
        virtual void pointerUp( float inX, float inY );


        char mHover;
        double mWide, mHigh;
        
        char isInside( float inX, float inY );


        SpriteHandle mOnSprite;
        SpriteHandle mOffSprite;

        double mDrawScale;
        
    };



#endif
