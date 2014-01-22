#include "GamePage.h"

#include "TextField.h"
#include "TextButton.h"
#include "KeyEquivalentTextButton.h"


#include "minorGems/ui/event/ActionListener.h"


class LoginPage : public GamePage, public ActionListener {
        
    public:
        
        LoginPage();
        
        virtual ~LoginPage();

        
        char getLoginDone();
        

        virtual void actionPerformed( GUIComponent *inTarget );
        


        virtual void step();
        
        virtual void draw( doublePair inViewCenter, 
                           double inViewSize );
        
        virtual void makeActive( char inFresh );
        virtual void makeNotActive();
        

        // for TAB and ENTER (switch fields and start login)
        virtual void keyDown( unsigned char inASCII );
        
        // for arrow keys (switch fields)
        virtual void specialKeyDown( int inKeyCode );
        
    protected:
        
        TextField mEmailField;
        TextField mTicketField;

        TextField *mFields[2];

        TextButton mAtSignButton;

        KeyEquivalentTextButton mPasteButton;
        
        TextButton mLoginButton;


        char mHaveServerURL;
        char mLoggedIn;
        
        // if client out of date
        char mLoginBlocked;
        

        char *mServerURL;

        int mRequestSteps;
        int mWebRequest;


        void switchFields();
        
        // set inFreshLogin to false if continuing a subsequent
        // step in the same login action (so that we don't insert
        // a pause again)
        void startLogin( char inFreshLogin=true );
        

        void acceptInput();

    };

