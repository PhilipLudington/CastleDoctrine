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
        virtual void clearToolTip( const char *inTipToClear );
        

        
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

        
        // override to catch keypress to dismiss shutdown warning
        // overlay
        virtual void base_keyDown( unsigned char inASCII );

        
    protected:
        


        // subclasses override these to provide custom functionality

        virtual void step() {
            };
        
        // called before added sub-components have been drawn
        virtual void drawUnderComponents( doublePair inViewCenter, 
                                          double inViewSize ) {
            };
        // called after added sub-components have been drawn
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
        

        // subclasses can override this to selectively permit
        // the waiting icon to be drawn at particular times
        virtual char canShowWaitingIcon() {
            return true;
            }
        
        // subclasses can override this to control size of waiting icon
        // displayed on their pages
        virtual char makeWaitingIconSmall() {
            return false;
            }
        


        // override this from PageComponent to show waiting status
        virtual void setWaiting( char inWaiting,
                                 char inWarningOnly = false );
        


        // shows an overlay message warning the user that
        // a server shutdown is pending
        virtual void showShutdownPendingWarning();
        

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
        static char sShowWaitingWarningOnly;
        
        static char sShutdownPendingWarning;
    };


#endif

