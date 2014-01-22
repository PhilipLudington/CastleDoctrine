#include "LiveHousePage.h"

#include "TextButton.h"

#include "BlueprintHouseGridDisplay.h"

#include "minorGems/ui/event/ActionListener.h"




class ViewBlueprintPage : public LiveHousePage, public ActionListener {
        
    public:
        
        ViewBlueprintPage();
        
        virtual ~ViewBlueprintPage();
        
        
        // call before making active the first time
        //
        // set to true to flag this page as part of a live house checkout
        // where pings should be sent in response to user activity
        // If false, pings are never sent.
        //
        // Defaults to false.
        void setLive( char inLive );
        

        
        // destroyed by caller
        void setHouseMap( const char *inHouseMap );

        void setDescription( const char *inDescription );

        void setViewOffset( int inXOffset, int inYOffset );
        

        char getDone() {
            return mDone;
            }
            

        virtual void actionPerformed( GUIComponent *inTarget );


        virtual void draw( doublePair inViewCenter, 
                           double inViewSize );
        
        virtual void step();
        
        
        virtual void makeActive( char inFresh );

    protected:


        BlueprintHouseGridDisplay mGridDisplay;
        TextButton mDoneButton;
        
        char mDone;
        
        char *mDescription;

        char mLive;
    };

