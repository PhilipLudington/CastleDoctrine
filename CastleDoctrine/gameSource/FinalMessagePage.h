#include "GamePage.h"

#include "minorGems/ui/event/ActionListener.h"


class FinalMessagePage : public GamePage {
        
    public:

        FinalMessagePage();
        
        ~FinalMessagePage();

        // must be a string literal
        void setMessageKey( const char *inKey );

        // copied internally
        // set to NULL to clear
        void setSubMessage( const char *inSubMessage );

        virtual void makeActive( char inFresh );

        virtual void draw( doublePair inViewCenter, 
                           double inViewSize );

    protected:
        const char *mKey;

        char *mSubMessage;
        
    };

