#include "RobberyReplayMenuPage.h"

#include "minorGems/game/game.h"


extern Font *mainFont;





RobberyReplayMenuPage::RobberyReplayMenuPage() 
        : mPickList( 0, 0, true, this ),
          mMenuButton( mainFont, 4, -4, translate( "returnMenu" ) ),
          mReplayButton( mainFont, -4, -4, translate( "startReplay" ) ),
          mEditHouseButton( mainFont, 4, -4, translate( "returnHome" ) ),
          mReturnToMenu( false ),
          mStartReplay( false ),
          mStartEditHouse( false ) {

    addComponent( &mMenuButton );
    addComponent( &mReplayButton );
    addComponent( &mEditHouseButton );
    
    addComponent( &mPickList );
    

    mMenuButton.addActionListener( this );
    mReplayButton.addActionListener( this );
    mEditHouseButton.addActionListener( this );

    mPickList.addActionListener( this );

    mReplayButton.setVisible( false );
    mEditHouseButton.setVisible( false );
    }


        
RobberyReplayMenuPage::~RobberyReplayMenuPage() {
    }



void RobberyReplayMenuPage::setEditHouseOnDone( char inEdit ) {
    if( inEdit ) {
        mEditHouseButton.setVisible( true );
        mMenuButton.setVisible( false );
        }
    else {
        mEditHouseButton.setVisible( false );
        mMenuButton.setVisible( true );
        }
    }



char RobberyReplayMenuPage::getReturnToMenu() {
    return mReturnToMenu;
    }



char RobberyReplayMenuPage::getStartReplay() {
    return mStartReplay;
    }



char RobberyReplayMenuPage::getStartEditHouse() {
    return mStartEditHouse;
    }



int RobberyReplayMenuPage::getLogID() {
    HouseRecord *record = mPickList.getSelectedHouse();

    if( record == NULL ) {
        return -1;
        }
    else {
        return record->uniqueID;
        }
    }



void RobberyReplayMenuPage::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == &mMenuButton ) {
        mReturnToMenu = true;
        }
    else if( inTarget == &mEditHouseButton ) {
        mStartEditHouse = true;
        }
    else if( inTarget == &mPickList ) {
        if( mPickList.getSelectedHouse() == NULL ) {
            mReplayButton.setVisible( false );
            }
        else {
            mReplayButton.setVisible( true );
            }
        }
    else if( inTarget == &mReplayButton ) {
        mStartReplay = true;
        }
    
    }



void RobberyReplayMenuPage::step() {
    }


        
void RobberyReplayMenuPage::draw( doublePair inViewCenter, 
                          double inViewSize ) {
    }


        
void RobberyReplayMenuPage::makeActive( char inFresh ) {
    if( !inFresh ) {
        return;
        }
    
    mPickList.refreshList( true, true );
    
    mReturnToMenu = false;
    mStartReplay = false;
    mStartEditHouse = false;

    mStatusMessageKey = NULL;
    mStatusError = false;
    }


        

