#include "PaymentReportPage.h"

#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"

#include "minorGems/util/stringUtils.h"

#include "message.h"


extern Font *mainFont;


PaymentReportPage::PaymentReportPage() 
        : mDoneButton( mainFont, 4, -4, 
                       translate( "okay" ) ), 
          mDone( false ),
          mWifeName( NULL ) {    

    addComponent( &mDoneButton );
    mDoneButton.addActionListener( this );
    }


PaymentReportPage::~PaymentReportPage() {
    if( mWifeName != NULL ) {
        delete [] mWifeName;
        }
    }




void PaymentReportPage::setPayments( const char *inWifeName, 
                                     int inPaymentCount, int inYouPaidTotal,
                                    int inWifePaidTotal  ) {
    
    if( mWifeName != NULL ) {
        delete [] mWifeName;
        }
    mWifeName = stringDuplicate( inWifeName );

    mPaymentCount = inPaymentCount;
    mYouPaidTotal = inYouPaidTotal;
    mWifePaidTotal = inWifePaidTotal;
    }




char PaymentReportPage::getDone() {
    return mDone;
    }



void PaymentReportPage::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == &mDoneButton ) {
        mDone = true;
        }
    }



void PaymentReportPage::makeActive( char inFresh ) {
    if( !inFresh ) {
        return;
        }
    
    mDone = false;
    
    mStatusError = false;
    }



void PaymentReportPage::draw( doublePair inViewCenter, 
                              double inViewSize ) {

    doublePair labelPos = { 0, 1 };

    const char *headingKey = "workReport";
    if( mPaymentCount > 1 ) {
        headingKey = "workReportPlural";
        }

    char *headingString = autoSprintf( translate( headingKey ),
                                       mPaymentCount );
    
    drawMessage( headingString, labelPos, false ); 

    delete [] headingString;
    

    char *youString = autoSprintf( translate( "youEarnReport" ),
                                   mYouPaidTotal );
    
    labelPos.y = 0;
    
    drawMessage( youString, labelPos, false ); 

    delete [] youString;
    

    if( mWifePaidTotal > 0 ) {
        
        char *wifeString = autoSprintf( translate( "wifeEarnReport" ),
                                        mWifeName, mWifePaidTotal );
        
        labelPos.y = -1;
        
        drawMessage( wifeString, labelPos, false ); 
        
        delete [] wifeString;
        }
    }


