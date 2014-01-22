#include "GamePage.h"

#include "TextField.h"
#include "TextButton.h"
#include "RobPickList.h"

#include "minorGems/util/SimpleVector.h"


#include "minorGems/ui/event/ActionListener.h"




class MenuPage : public GamePage, public ActionListener {
        
    public:
        
        MenuPage();
        
        virtual ~MenuPage();


        virtual char getStartEditHouse();
        virtual char getStartRobHouse();
        virtual char getViewBlueprint();
        virtual char getShowReplayList();
        virtual char getStartSelfTestReplay();
        
        // destroyed internally
        // can be NULL
        virtual HouseRecord *getSelectedHouse();
        

        virtual void actionPerformed( GUIComponent *inTarget );


        virtual void step();
        
        virtual void draw( doublePair inViewCenter, 
                           double inViewSize );
        
        virtual void makeActive( char inFresh );

    protected:
        

        RobPickList mPickList;

        TextButton mEditHouseButton;
        TextButton mRobHouseButton;
        
        TextButton mShowReplayListButton;
        TextButton mProofReplayButton;

        TextButton mBlueprintButton;
        

        char mStartEditHouse;
        char mStartRobHouse;
        char mViewBlueprint;
        char mShowReplayList;
        char mStartSelfTestReplay;
        
    };

