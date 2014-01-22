#include "GamePage.h"

#include "TextField.h"
#include "TextButton.h"

#include "minorGems/ui/event/ActionListener.h"


class StaleHousePage : public GamePage, public ActionListener {
        
    public:
        
        StaleHousePage( char inDead );
        

        void setOutOfTime( char inOutOfTime );
        
        
        virtual char getDone();
        

        virtual void actionPerformed( GUIComponent *inTarget );
        

        virtual void makeActive( char inFresh );



    protected:

        TextButton mDoneButton;

        char mDone;
        
        char mDead;
        
        char mOutOfTime;
    };

