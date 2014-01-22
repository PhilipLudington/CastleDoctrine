#include "GamePage.h"

#include "TextField.h"
#include "TextButton.h"
#include "GallerySlotButton.h"

#include "minorGems/ui/event/ActionListener.h"


class BuyAuctionPage : public GamePage, public ActionListener {
        
    public:
        
        BuyAuctionPage();
        
        virtual ~BuyAuctionPage();


        virtual char getReturnToHome();
                

        virtual void setObject( int inObjectID );
        
        virtual void setLootValue( int inLootValue );

        virtual int getLootValue();
        

        // destroyed by caller
        virtual void setGalleryContents( const char *inGalleryContents );
        virtual char *getGalleryContents();
        
        

        

        virtual void actionPerformed( GUIComponent *inTarget );
        


        virtual void step();
        
        virtual void draw( doublePair inViewCenter, 
                           double inViewSize );
        
        virtual void makeActive( char inFresh );


    protected:

        int mWebRequest;

        int mObjectID;
        char *mGalleryContents;
        int mLootValue;

        char mDone;

        TextButton mHomeButton;
        
        
        GallerySlotButton mBoughtSlot;
        
        char mReturnToHome;
        
    };

