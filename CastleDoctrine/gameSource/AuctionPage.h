#include "LiveHousePage.h"

#include "TextButton.h"
#include "GallerySlotButton.h"

#include "minorGems/util/SimpleVector.h"


#include "minorGems/ui/event/ActionListener.h"



#define NUM_AUCTION_SLOTS  12

#define NUM_AUCTION_SLOTS_PER_ROW  4


class AuctionPage : public LiveHousePage, public ActionListener {
        
    public:
        
        AuctionPage();
        
        virtual ~AuctionPage();


        virtual char getDone();

        virtual char getBuy();
        
        int getBoughtObject();
        

        void setLootValue( int inLootValue );

        virtual void actionPerformed( GUIComponent *inTarget );


        virtual void step();
        
        virtual void draw( doublePair inViewCenter, 
                           double inViewSize );
        
        virtual void makeActive( char inFresh );

    protected:
        TextButton mDoneButton;
        TextButton mUpdateButton;
        TextButton mBuyButton;
        
        GallerySlotButton *mAuctionSlots[ NUM_AUCTION_SLOTS ];

        int mAuctionPrices[ NUM_AUCTION_SLOTS ];

        
        int mDisplayOffset;
        SimpleVector<int> mFullIDList;
        SimpleVector<int> mFullPriceList;

        SpriteButton mUpButton;
        SpriteButton mDownButton;

        int mLootValue;
        
        int mWebRequest;

        int mSecondsUntilUpdate;
        int mBaseTimestamp;

        char mDone;
        char mBuyExecuted;
        int mBoughtObjectID;

        // after buy button pressed, force refresh on next display
        // whether or not prices are stale
        char mForceRefresh;
        

        void refreshPrices();

        void turnAllRingsOff();

        char getPricesStale();

        void populateSlots();
        
        void setUpDownVisibility();
    };

