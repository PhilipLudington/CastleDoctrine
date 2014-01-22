#include "ViewBlueprintPage.h"

#include "message.h"
#include "balance.h"


#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"
#include "minorGems/game/drawUtils.h"

#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SettingsManager.h"



extern Font *mainFont;


extern int diffHighlightsOff;



ViewBlueprintPage::ViewBlueprintPage() 
        : mGridDisplay( 0, 0 ),
          mDoneButton( mainFont, 8, -2.5, translate( "doneEdit" ) ),
          mDone( false ),
          mDescription( NULL ),
          mLive( false ) {

    addComponent( &mDoneButton );
    addComponent( &mGridDisplay );

    mDoneButton.addActionListener( this );
    mGridDisplay.addActionListener( this );
    }


        
ViewBlueprintPage::~ViewBlueprintPage() {
    if( mDescription != NULL ) {
        delete [] mDescription;
        }
    }


void ViewBlueprintPage::setLive( char inLive ) {
    mLive = inLive;
    }


void ViewBlueprintPage::setHouseMap( const char *inHouseMap ) {
    mGridDisplay.setHouseMap( inHouseMap );
    }


void ViewBlueprintPage::setDescription( const char *inDescription ) {
    if( mDescription != NULL ) {
        delete [] mDescription;
        }
    mDescription = stringDuplicate( inDescription );
    }


void ViewBlueprintPage::setViewOffset( int inXOffset, int inYOffset ) {
    mGridDisplay.setVisibleOffset( inXOffset, inYOffset );
    }

    

void ViewBlueprintPage::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == &mDoneButton ) {
        mDone = true;
        }
    else if( inTarget == &mGridDisplay ) {
        // movement on blueprint

        if( mLive ) {
            actionHappened();
            }
        }
    }





        
void ViewBlueprintPage::makeActive( char inFresh ) {
    if( mLive ) {
        LiveHousePage::makeActive( inFresh );
        }
    
    if( !inFresh ) {
        return;
        }
    mDone = false;
    }
        

extern Font *numbersFontFixed;


void ViewBlueprintPage::draw( doublePair inViewCenter, 
                              double inViewSize ) {
    
    if( mDescription != NULL ) {
        doublePair labelPos = { 0, 6.75 };
        
        if( strstr( mDescription, "##" ) != NULL ) {
            // two lines, move up a bit
            labelPos.y = 7;
            }

        drawMessage( mDescription, labelPos );
        }
    }




void ViewBlueprintPage::step() {
    if( mLive ) {
        LiveHousePage::step();
        }
    
    }

