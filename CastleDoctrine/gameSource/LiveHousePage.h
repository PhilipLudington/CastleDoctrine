#ifndef LIVE_HOUSE_PAGE_INCLUDED
#define LIVE_HOUSE_PAGE_INCLUDED



#include "GamePage.h"



// base class for live house checkouts that handles pinging house on server
// to keep it checked out
class LiveHousePage : public GamePage {
        
    public:
        
        LiveHousePage();
        
        virtual ~LiveHousePage();
        
        virtual void step();

        virtual void makeActive( char inFresh );

        virtual char isStale() {
            return mCheckoutStale;
            }

        virtual char didStartTestFail() {
            return mStartTestFailed;
            }

        virtual char isOutOfTime() {
            return mOutOfTime;
            }
        
        // tells server that owner is self testing house
        virtual void startSelfTest();

        // tells server that owner is done self testing house
        virtual void endSelfTest();
        

        // track this globally
        // User only has one house checked out at a time, but they
        // might move from Page to Page while having that same house checked
        // out.  
        static int sLastPingTime;

        

    protected:

        
        // subclasses should call this every time user does something
        // substatial (e.g., place object in house) to show that they are
        // still present and in the game
        virtual void actionHappened();
        
        // true if we have any request that we're waiting for responses
        // from or any queued up to be sent
        char areRequestsPending();


    private:



        // subclass should update this every time a user action happens
        int mLastActionTime;


        // becomes true when a ping fails
        char mCheckoutStale;
        
        // futher marks stale checkout as an out-of-time
        char mOutOfTime;
        

        // true if starting a self test failed
        char mStartTestFailed;
        
        char mCurrentRequestForStartTest;


        // static, because one page might need to finish up a request
        // started by a previous, now-closed page
        static int sWebRequest;


        // a fifo queue of pending Start- and EndSelfTest requests
        static SimpleVector<const char *> sPendingTestRequests;

        
        
    };



#endif
