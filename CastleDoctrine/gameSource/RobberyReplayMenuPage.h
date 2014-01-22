#include "GamePage.h"

#include "TextField.h"
#include "TextButton.h"
#include "RobPickList.h"

#include "minorGems/util/SimpleVector.h"


#include "minorGems/ui/event/ActionListener.h"




class RobberyReplayMenuPage : public GamePage, public ActionListener {
        
    public:
        
        RobberyReplayMenuPage();
        
        virtual ~RobberyReplayMenuPage();


        // set to true to toggle returning to house editing after done
        virtual void setEditHouseOnDone( char inEdit );
        

        
        virtual char getReturnToMenu();
        virtual char getStartReplay();
        virtual char getStartEditHouse();

        virtual int getLogID();
        

        virtual void actionPerformed( GUIComponent *inTarget );


        virtual void step();
        
        virtual void draw( doublePair inViewCenter, 
                           double inViewSize );
        
        virtual void makeActive( char inFresh );

    protected:
        

        RobPickList mPickList;

        TextButton mMenuButton;
        TextButton mReplayButton;
        
        TextButton mEditHouseButton;
        

        char mReturnToMenu;
        char mStartReplay;
        char mStartEditHouse;
        
    };

