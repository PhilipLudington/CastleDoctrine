#include "GamePage.h"

#include "minorGems/ui/event/ActionListener.h"


class FinalMessagePage : public GamePage {
        
    public:

        FinalMessagePage();
        

        void setMessageKey( const char *inKey );
        

        virtual void makeActive( char inFresh );

        virtual void draw( doublePair inViewCenter, 
                           double inViewSize );

    protected:
        const char *mKey;
        
    };

