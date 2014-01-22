#ifndef GAME_PAGE_INCLUDED
#define GAME_PAGE_INCLUDED


#include "minorGems/game/doublePair.h"
#include "minorGems/game/gameGraphics.h"

#include "PageComponent.h"


class GamePage : public PageComponent {
        

    public:
        

        virtual ~GamePage();
        
        void setStatus( const char *inStatusMessageKey, char inError );

        // inStatusMessage destroyed by caller
        void setStatusDirect( char *inStatusMessage, char inError );

        char isStatusShowing();
        

        // overrides default tip position
        // tip defaults to bottom of screen
        void setTipPosition( char inTop );
        
        
        // override these from PageComponent to actually SHOW
        // the tool tip, instead of passing it further up the parent chain
        // inTip can either be a translation key or a raw tip
        // copied internally
        virtual void setToolTip( const char *inTip );
        

        
        // inFresh set to true when returning to this page
        // after visiting other pages
        // set to false after returning from pause.
        void base_makeActive( char inFresh );
        void base_makeNotActive();


        // override to draw status message
        virtual void base_draw( doublePair inViewCenter, 
                                double inViewSize );


        // override to step waiting display
        virtual void base_step();

        
    protected:
        


        // subclasses override these to provide custom functionality

        virtual void step() {
            };
        
        virtual void draw( doublePair inViewCenter, 
                           double inViewSize ) {
            };
        
        // inFresh set to true when returning to this page
        // after visiting other pages
        // set to false after returning from pause.
        virtual void makeActive( char inFresh ) {
            };
        virtual void makeNotActive() {
            };


        virtual void pointerMove( float inX, float inY );
        

        virtual void pointerDown( float inX, float inY ) {
            };

        virtual void pointerDrag( float inX, float inY ) {
            pointerMove( inX, inY );
            };

        virtual void pointerUp( float inX, float inY ) {
            };

        virtual void keyDown( unsigned char inASCII ) {
            };
        
        virtual void keyUp( unsigned char inASCII ) {
            };

        virtual void specialKeyDown( int inKeyCode ) {
            };

        virtual void specialKeyUp( int inKeyCode ) {
            };
        

        // override this from PageComponent to show waiting status
        virtual void setWaiting( char inWaiting );
        
        GamePage();
        

        char mStatusError;
        const char *mStatusMessageKey;
        char *mStatusMessage;
        

        char *mTip;
        char *mLastTip;
        double mLastTipFade;
        
        char mTipAtTopOfScreen;

        static int sPageCount;

        static SpriteHandle sWaitingSprites[3];
        static SpriteHandle sResponseWarningSprite;
        
        static int sCurrentWaitingSprite;
        static int sLastWaitingSprite;
        static int sWaitingSpriteDirection;
        static double sCurrentWaitingSpriteFade;
        
        static char sResponseWarningShowing;
        static doublePair sResponseWarningPosition;

        char mResponseWarningTipShowing;
        
        static double sWaitingFade;
        static char sWaiting;
    };


#endif

