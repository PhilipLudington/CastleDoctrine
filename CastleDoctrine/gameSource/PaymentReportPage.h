#include "GamePage.h"

#include "TextField.h"
#include "TextButton.h"

#include "minorGems/ui/event/ActionListener.h"


class PaymentReportPage : public GamePage, public ActionListener {
        
    public:
        
        PaymentReportPage();

        ~PaymentReportPage();
        

        void setPayments( const char *inWifeName,
                          int inPaymentCount, int inYouPaidTotal,
                          int inWifePaidTotal  );
        

        virtual char getDone();
        

        virtual void actionPerformed( GUIComponent *inTarget );
        

        virtual void makeActive( char inFresh );

        virtual void draw( doublePair inViewCenter, 
                           double inViewSize );

    protected:

        TextButton mDoneButton;

        char mDone;

        char *mWifeName;
        int mPaymentCount;
        int mYouPaidTotal;
        int mWifePaidTotal;
    };

