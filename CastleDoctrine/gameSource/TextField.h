#ifndef TEXT_FIELD_INCLUDED
#define TEXT_FIELD_INCLUDED


#include "minorGems/game/Font.h"
#include "minorGems/ui/event/ActionListenerList.h"

#include "PageComponent.h"


// fires action performed when ENTER hit inside field
class TextField : public PageComponent, public ActionListenerList {
        
    public:
        
        // Fixed width font is used for sizing of text field
        // centered on inX, inY

        // label text and char maps copied internally
        TextField( Font *inFixedFont, 
                   Font *inDisplayFont, 
                   double inX, double inY, int inCharsWide,
                   char inForceCaps = false,
                   const char *inLabelText = NULL,
                   const char *inAllowedChars = NULL,
                   const char *inForbiddenChars = NULL );

        virtual ~TextField();

        // copied internally
        void setText( const char *inText );
        

        // destroyed by caller
        char *getText();
        
        // at current cursor position
        void insertCharacter( unsigned char inASCII );


        virtual void setActive( char inActive );
        virtual char isActive();
        
        
        
        virtual void step();
        
        virtual void draw();

        virtual void pointerUp( float inX, float inY );

        virtual void keyDown( unsigned char inASCII );
        virtual void keyUp( unsigned char inASCII );
        
        virtual void specialKeyDown( int inKeyCode );
        virtual void specialKeyUp( int inKeyCode );
        

        // makes this text field the only focused field.
        // causes it to respond to keystrokes that are passed to it
        virtual void focus();
        
        // if this field is the only focused field, this causes no
        // fields to be focused.
        // if this field is currently unfocused, this call has no effect.
        virtual void unfocus();

        virtual char isFocused();

        // defaults to 30 and 2
        static void setDeleteRepeatDelays( int inFirstDelaySteps,
                                           int inNextDelaySteps );
        

        
    protected:
        char mActive;
        
        Font *mFont;
        int mCharsWide;

        char mForceCaps;

        char *mLabelText;
        
        char *mAllowedChars;
        char *mForbiddenChars;

        
        double mWide, mHigh;
        
        double mBorderWide;
        
        // width of a single character
        double mCharWidth;
        
        

        char mFocused;

        char *mText;

        int mCursorPosition;
        

        int mHoldDeleteSteps;
        char mFirstDeleteRepeatDone;

        int mHoldArrowSteps[2];
        char mFirstArrowRepeatDone[2];
        

        void deleteHit();
        void leftHit();
        void rightHit();
        
        void clearArrowRepeat();
        
        
        // returns 0 if character completely forbidden by field rules
        unsigned char processCharacter( unsigned char inASCII );
        
        

        // clever (!) way of handling focus?

        // keep static pointer to focused field (there can be only one)
        static TextField *sFocusedTextField;
        

        static int sDeleteFirstDelaySteps;
        static int sDeleteNextDelaySteps;
        
    };


#endif
