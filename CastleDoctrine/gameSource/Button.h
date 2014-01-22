#ifndef BUTTON_INCLUDED
#define BUTTON_INCLUDED


#include "PageComponent.h"

#include "minorGems/game/Font.h"
#include "minorGems/ui/event/ActionListenerList.h"


// button superclass that draws a 1-pixel border and handles events
// fires actionPerformed whenever button pressed
class Button : public PageComponent, public ActionListenerList {
        
    public:
        
        // centered on inX, inY
        Button( double inX, double inY,
                double inWide, double inHigh,
                double inPixelSize );

        virtual ~Button();

        // set the tool tip that will be passed up the parent chain
        // when this button is moused over
        // NULL disables tip display for this button
        // copied internally
        virtual void setMouseOverTip( const char *inTipMessage );
        
        
        // overrides to clear state when made invisible
        virtual void setVisible( char inIsVible );

        
        virtual double getWidth();

        
    protected:

        virtual void clearState();
        
        virtual void step();
        
        virtual void draw();

        virtual void pointerMove( float inX, float inY );
        virtual void pointerDown( float inX, float inY );
        virtual void pointerDrag( float inX, float inY );
        
        // fires action performed to listener list
        virtual void pointerUp( float inX, float inY );        


        char mHover;
        char mDragOver;

        double mWide, mHigh, mPixWidth;
        
        char *mMouseOverTip;


        char isInside( float inX, float inY );


        // draw the contents of the button
        // should be overridden by subclasses
        // Button class sets the draw color before calling drawContents
        // (default implementation draws nothing)
        virtual void drawContents();
        
        
        // draws the border of the button
        // default is just a rectangle
        // Button class sets the draw color before calling drawBorder
        virtual void drawBorder();
        
        
    };



#endif
