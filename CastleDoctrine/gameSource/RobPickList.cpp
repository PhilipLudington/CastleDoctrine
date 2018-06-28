#include "RobPickList.h"

#include "serialWebRequests.h"


#include "ticketHash.h"

#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"
#include "minorGems/game/drawUtils.h"

#include "minorGems/util/stringUtils.h"


extern Font *mainFont;
extern Font *mainFontFixed;

extern char *serverURL;

extern int userID;

extern double frameRateFactor;


static const int linesPerPage = 8;
static double lineHeight = 0.75;
static double lineWidthLeft = 7.25;
static double lineWidthRight = 7.25;


static double topOffset = ( linesPerPage * lineHeight ) / 2 - lineHeight / 2;



RobPickList::RobPickList( double inX, double inY,
                          char inRobberyLog,
                          GamePage *inParentPage )
        : PageComponent( inX, inY ),
          mParentPage( inParentPage ),
          mArePagesLeft( 0 ),
          mCurrentSkip( 0 ),
          mWebRequest( -1 ),
          mProgressiveDrawSteps( 0 ),
          mRobberyLog( inRobberyLog ),
          mSkullSprite( loadSprite( "skull.tga" ) ),
          mHover( false ),
          mUpButton( "up.tga", 8.25, 1, 1/16.0 ),
          mDownButton( "down.tga", 8.25, -1, 1/16.0 ),
          mSearchField( mainFontFixed, mainFont, 
                        0, 4,
                        8,
                        false,
                        translate( "nameSearch" ),
                        "abcdefghijklmnopqrstuvwxyz"
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                        " " ),
          mFilterButton( mainFont, 4, 4,
                         translate( "filter" ) ),
          mIgnoreButton( mainFont, -4, 6,
                         translate( "ignoreHouse" ) ),
          mClearIgnoreListButton( mainFont, 4, 6,
                                  translate( "clearIgnoreList" ) ),
          mAppliedSearchWords( stringDuplicate( "" ) ),
          mIgnoreSet( false ),
          mIgnoreTarget( NULL ),
          mAnyPossiblyIgnored( true ),
          mClearIgnoreListSet( false ){

    mUpButton.setVisible( false );
    mDownButton.setVisible( false );
    
    mIgnoreButton.setVisible( false );
    mClearIgnoreListButton.setVisible( false );

    addComponent( &mUpButton );
    addComponent( &mDownButton );
    addComponent( &mSearchField );
    addComponent( &mFilterButton );
    addComponent( &mIgnoreButton );
    addComponent( &mClearIgnoreListButton );
    
    mUpButton.addActionListener( this );
    mDownButton.addActionListener( this );

    mSearchField.addActionListener( this );
    mFilterButton.addActionListener( this );
    mIgnoreButton.addActionListener( this );
    mClearIgnoreListButton.addActionListener( this );

    mIgnoreButton.setMouseOverTip( translate( "ignoreTip" ) );
    mClearIgnoreListButton.setMouseOverTip( 
        translate( "clearIgnoreListTip" ) );

    if( !mRobberyLog ) {
        // start off with no skip specified to fetch house page
        // near our value range from server first time
        mCurrentSkip = -1;
        }
    }



RobPickList::~RobPickList() {
    if( mWebRequest != -1 ) {
        clearWebRequestSerial( mWebRequest );
        }
    clearHouseList();

    delete [] mAppliedSearchWords;

    freeSprite( mSkullSprite );

    if( mIgnoreTarget != NULL ) {
        delete [] mIgnoreTarget;
        }
    }



void RobPickList::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == &mUpButton ) {
        mCurrentSkip -= linesPerPage;
        if( mCurrentSkip < 0 ) {
            mCurrentSkip = 0;
            }

        refreshList( true, true );
        }
    else if( inTarget == &mDownButton ) {
        mCurrentSkip += linesPerPage;

        refreshList( true, true );
        }
    else if( inTarget == &mFilterButton || inTarget == &mSearchField ) {
        delete [] mAppliedSearchWords;

        char *searchText = mSearchField.getText();

        SimpleVector<char *> *words = tokenizeString( searchText );

        delete [] searchText;
        
        if( words->size() == 0 ) {
            // reset us back to our neighborhood on the house list
            mCurrentSkip = -1;
            }
        
        if( words->size() > 0 ) {
            char **wordsArray = words->getElementArray();
        
            mAppliedSearchWords = join( wordsArray, words->size(), "+" );

            for( int i=0; i<words->size(); i++ ) {
                delete [] wordsArray[i];
                }
            delete [] wordsArray;
            }
        else {
            mAppliedSearchWords = stringDuplicate( "" );
            }

        delete words;

        
        refreshList( true, false );
        }
    else if( inTarget == &mIgnoreButton ) {
        mIgnoreSet = true;
        if( mIgnoreTarget != NULL ) {
            delete [] mIgnoreTarget;
            }
        mIgnoreTarget = 
            stringDuplicate( getSelectedHouse()->rawCharacterName );
        
        mAnyPossiblyIgnored = true;
        
        refreshList( true, true );
        }
    else if( inTarget == &mClearIgnoreListButton ) {
        mClearIgnoreListSet = true;
        
        // those that we've ignored have been cleared now
        mAnyPossiblyIgnored = false;
        
        mClearIgnoreListButton.setVisible( false );
        
        // back to default list position, 
        // since all bets are off about positioning
        // with all the new list members that might be added
        if( strcmp( mAppliedSearchWords, "" ) == 0 ) {
            // no filter words
            // reset us back to our neighborhood on the house list
            mCurrentSkip = -1;
            }

        refreshList( true, false );
        }
    }



// fetch new results from server
void RobPickList::refreshList( char inPreserveSearch,
                               char inPreservePosition ) {
    if( mWebRequest != -1 ) {
        clearWebRequestSerial( mWebRequest );
        }
    clearHouseList();

    mUpButton.setVisible( false );
    mDownButton.setVisible( false );
    mIgnoreButton.setVisible( false );

    if( !mRobberyLog && mAnyPossiblyIgnored ) {
        mClearIgnoreListButton.setVisible( true );
        }
    
    mSearchField.focus();

    if( ! inPreserveSearch ) {
        delete [] mAppliedSearchWords;
        mAppliedSearchWords = stringDuplicate( "" );
        mSearchField.setText( mAppliedSearchWords );
        }
    
    if( ! inPreservePosition && mCurrentSkip != -1 ) {
        // reset list, and not already resetting it to our neighborhood
        mCurrentSkip = 0;
        }


    // request house list from server
    char *ticketHash = getTicketHash();
        
    const char *action = "list_houses";
    
    if( mRobberyLog ) {
        action = "list_logged_robberies";
        }
        
    char *ignoreParameter;
    
    if( mIgnoreSet ) {
        ignoreParameter = autoSprintf( "&add_to_ignore_list=%s",
                                       mIgnoreTarget );
        mIgnoreSet = false;
        }
    else {
        ignoreParameter = stringDuplicate( "" );
        }


    char *clearIgnoreListParameter;
    
    if( mClearIgnoreListSet ) {
        clearIgnoreListParameter = stringDuplicate( "&clear_ignore_list=1" );
        mClearIgnoreListSet = false;
        }
    else {
        clearIgnoreListParameter = stringDuplicate( "" );
        }
    
    char *skipString;
    
    if( mCurrentSkip != -1 ) {
        skipString = autoSprintf( "&skip=%d", mCurrentSkip );
        }
    else {
        skipString = stringDuplicate( "" );
        }
    

    char *actionString = autoSprintf( 
        "action=%s%s&limit=%d&name_search=%s%s%s&user_id=%d"
        "&%s",
        action, skipString, linesPerPage, mAppliedSearchWords,
        ignoreParameter, clearIgnoreListParameter,
        userID, ticketHash );

    delete [] skipString;
    delete [] ticketHash;
    delete [] ignoreParameter;
    delete [] clearIgnoreListParameter;
            
    
    mWebRequest = startWebRequestSerial( "POST", 
                                   serverURL, 
                                   actionString );
    
    delete [] actionString;

    // nothing selected now (so selection might have changed)
    fireActionPerformed( this );
    
    setWaiting( true );
    }



HouseRecord *RobPickList::getSelectedHouse() {
    for( int i=0; i<mHouseList.size(); i++ ) {
        HouseRecord *r = mHouseList.getElement( i );
        
        if( r->selected ) {
            return r;
            }
        }

    return NULL;
    }







// inName destroyed internally
// result destroyed by caller
static char *trimName( char *inName, double inMaxLength ) {
    
    // make sure name isn't too long for display
    if( mainFont->measureString( inName ) 
        > inMaxLength ) {
                            
        // try trimming middle name
        int namePartCount;
        char **nameParts = split( inName, " ",
                                  &namePartCount );
                                
        // first letter only, followed by period
        nameParts[1][1] = '.';
        nameParts[1][2] = '\0';
                                
        delete [] inName;
                                
        inName = 
            join( nameParts, namePartCount, " " );
                                
        if( mainFont->measureString( inName ) 
            > inMaxLength ) {
            // still too long!
                                    
            // condense first name too.

            // first letter only, followed by period
            nameParts[0][1] = '.';
            nameParts[0][2] = '\0';
                                
            delete [] inName;
                                    
            inName = 
                join( nameParts, namePartCount, " " );
            }                                        

        for( int j=0; j<namePartCount; j++ ) {
            delete [] nameParts[j];
            }
        delete [] nameParts;
        }

    if( strcmp( inName, "You" ) == 0 ) {
        delete [] inName;
        
        inName = stringDuplicate( translate( "nameYou" ) );
        }
    

    return inName;
    }



void RobPickList::step() {
    if( mWebRequest != -1 ) {
            
        int stepResult = stepWebRequestSerial( mWebRequest );
        
        if( stepResult != 0 ) {
            setWaiting( false );
            }

        mProgressiveDrawSteps = 0;
        
        switch( stepResult ) {
            case 0:
                break;
            case -1:
                mParentPage->setStatus( "err_webRequest", true );
                clearWebRequestSerial( mWebRequest );
                mWebRequest = -1;
                break;
            case 1: {
                char *result = getWebResultSerial( mWebRequest );
                clearWebRequestSerial( mWebRequest );
                mWebRequest = -1;
                     
                printf( "Web result = %s\n", result );
   
                if( strstr( result, "DENIED" ) != NULL ) {
                    mParentPage->setStatus( "listFetchFailed", true );
                    }
                else {
                    // got house list!

                    mParentPage->setStatus( NULL, false );

                    // parse result
                    SimpleVector<char *> *lines = 
                        tokenizeString( result );



                    char badParse = false;

                    // last line should be OK
                    // second-to-last should be more_pages flat
                    for( int i=0; i<lines->size() - 2; i++ ) {
                        char *line = *( lines->getElement( i ) );
                        
                        int numParts;
                        char **parts = split( line, "#", &numParts );
                        
                        if( numParts != 7 ) {
                            printf( "Unexpected number of parts (%d) on house "
                                    "list line: %d\n", numParts, i );
                            badParse = true;
                            }
                        else {
                            HouseRecord r;
                            
                            r.selected = false;
                            r.draw = false;
                            r.position.x = 0;
                            r.position.y = topOffset - i * lineHeight;
                            
                            sscanf( parts[0], "%d", &( r.uniqueID ) );
                            
                            
                            char found;
                            
                            r.rawCharacterName = stringDuplicate( parts[1] );
                            
                            r.characterName = 
                                trimName(
                                    replaceAll( parts[1], "_", " ", &found ),
                                    lineWidthLeft - 0.375 );
                            
                            r.lastRobberName = 
                                trimName (
                                    replaceAll( parts[2], "_", " ", &found ),
                                    lineWidthLeft - 0.375 );
                            
                            if( strlen( parts[3] ) > 0 
                                && parts[3][0] == 'b' ) {
                                
                                r.bountyFlag = 1;
                                sscanf( parts[3], "b%d", &( r.lootValue ) );
                                }
                            else {
                                r.bountyFlag = 0;
                                sscanf( parts[3], "%d", &( r.lootValue ) );
                                }
                            
                            sscanf( parts[4], "%d", &( r.stat.eitherStat ) );
                            sscanf( parts[5], "%d", &( r.robberDeaths ) );
                            
                            sscanf( parts[6], "%d", &( r.flag ) );

                            mHouseList.push_back( r );
                            }
                        
                        for( int j=0; j<numParts; j++ ) {
                            delete [] parts[j];
                            }
                        delete [] parts;

                        
                        delete [] line;
                        }
                    
                    if( lines->size() > 1 ) {
                        
                        char *line = 
                            *( lines->getElement( lines->size() -1  ) );
                    
                        if( strcmp( line, "OK" ) != 0 ) {
                            badParse = true;
                            }
                        
                        delete [] line;

                        line = 
                            *( lines->getElement( lines->size() - 2  ) );
                    
                        int numRead = 
                            sscanf( line, "%d#%d", &( mArePagesLeft ), 
                                    &( mCurrentSkip ) );
                        if( numRead != 2 ) {
                            badParse = true;
                            }
                        
                        delete [] line;

                        if( mHouseList.size() == 0 && mCurrentSkip > 0 ) {
                            // displayed list empty
                            
                            // there might be more results pages up
                            // auto jump
                            actionPerformed( &mUpButton );
                            }
                        }
                    else {
                        // list could be empty, but it still must have OK
                        // at the end
                        badParse = true;

                        for( int i=0; i<lines->size(); i++ ) {
                            char *line = *( lines->getElement( i ) );
                            delete [] line;
                            }
                        }
                    
                    

                    delete lines;

                    
                    if( badParse ) {
                        clearHouseList();
                        mParentPage->setStatus( "listFetchFailed", true );
                        }
                    }
                        
                        
                delete [] result;
                }
                break;
            }
        }

    
    mProgressiveDrawSteps++;
    
    if( mProgressiveDrawSteps > 2 * frameRateFactor ) {
        mProgressiveDrawSteps = 0;

        // turn on one at a time
        char foundNew = false;
        for( int i=0; i<mHouseList.size() && !foundNew; i++ ) {
            HouseRecord *r = mHouseList.getElement( i );
            if( !r->draw ) {
                r->draw = true;
                foundNew = true;
                }
            }
        if( !foundNew ) {
            // all have been drawn now
            
            if( mHouseList.size() < linesPerPage || 
                mArePagesLeft == 0 ) {
                // on last page of list
                mDownButton.setVisible( false );
                }
            else {
                mDownButton.setVisible( true );
                }
            
            if( mCurrentSkip > 0 ) {
                mUpButton.setVisible( true );
                }
            else {
                mUpButton.setVisible( false );
                }
            }
        
        }
    
    }



void RobPickList::draw() {
    
    // alternate every other line
    char altColor = false;
    

    for( int i=0; i<mHouseList.size(); i++ ) {
        HouseRecord *r = mHouseList.getElement( i );
    

        if( r->draw ) {
            
            if( r->selected ) {
                if( !mRobberyLog && r->flag ) {
                    // chill selected
                    setDrawColor( 0.31, 0.125, 0.5, 1 );
                    }
                else {
                    // regular selected
                    setDrawColor( 0.35, 0.35, 0.15, 1 );
                    }
                }
            else {
                if( altColor ) {
                    setDrawColor( 0.2, 0.2, 0.2, 1 );
                    }
                else {
                    setDrawColor( 0.15, 0.15, 0.15, 1 );
                    }
                }

            drawRect( r->position.x - lineWidthLeft, 
                      r->position.y - lineHeight / 2, 
                      r->position.x + lineWidthRight, 
                      r->position.y + lineHeight / 2 );


            if( mRobberyLog ) {                
                if( r->selected ) {
                    if( r->flag == 1 ) {
                        // old selected, yellow
                        setDrawColor( 1, 1, 0, 1 );
                        }
                    else {
                        // new, selected, greenish (brighter)
                        setDrawColor( 0, 1, 0, 1 );
                        }
                    }
                else {
                    if( r->flag == 1 ) {
                        // old white
                        setDrawColor( 1, 1, 1, 1 );
                        }
                    else {
                        // new, green
                        setDrawColor( 0, 1, .25, 1 );
                        }
                    }
                }
            else {
                if( r->selected ) {
                    if( r->flag == 1 ) {
                        // chills, selected, purple
                        setDrawColor( 0.63, 0.25, 1, 1 );
                        }
                    else {
                        // non-chills selected, yellow
                        setDrawColor( 1, 1, 0, 1 );
                        }
                    }
                else {
                    if( r->flag == 1 ) {
                        // chills, purple
                        setDrawColor( 0.5, 0, 1, 1 );
                        }
                    else {
                        // non-chills, white
                        setDrawColor( 1, 1, 1, 1 );
                        }
                    }
                }

            
            char *nameToDraw = r->characterName;
            
        
            if( mRobberyLog ) {
                nameToDraw = r->lastRobberName;
                }
            doublePair drawPos = r->position;
            drawPos.x -= .25;
            mainFont->drawString( nameToDraw, drawPos, alignRight );
            

            if( r->bountyFlag ) {
                drawPos = r->position;
            
                drawPos.x += .5;
                
                drawSprite( mSkullSprite, drawPos, 1.0 / 16.0 );
                }


            char *lootString = autoSprintf( "$%d", r->lootValue );
        
            drawPos = r->position;
            drawPos.x += 1.25;
        
            mainFont->drawString( lootString, drawPos, alignLeft );

            delete [] lootString;


            char *attemptString = autoSprintf( "%d", r->stat.eitherStat );
        
            drawPos = r->position;
            drawPos.x += 5.25;
        
            mainFont->drawString( attemptString, drawPos, alignRight );

            delete [] attemptString;

            
            setDrawColor( 1, 0, 0, 1 );
            char *deathsString = autoSprintf( "%d", r->robberDeaths );
        
            drawPos = r->position;
            drawPos.x += 6.75;
        
            mainFont->drawString( deathsString, drawPos, alignRight );

            delete [] deathsString;


            altColor = ! altColor;
            }
        }

    if( mHouseList.size() == 0 && mWebRequest == -1 ) {
        // request done, and list still empty
        
        // empty list back from server.

        setDrawColor( 1, 1, 1, 1 );

        doublePair drawPos = { 0, 0 };
        
        mainFont->drawString( translate( "emptyList" ), drawPos, alignCenter );
        }
    
        
    }




HouseRecord *RobPickList::getHitRecord( float inX, float inY ) {
    for( int i=0; i<mHouseList.size(); i++ ) {
        HouseRecord *r = mHouseList.getElement( i );
    
        if( inX > - lineWidthLeft && inX < lineWidthRight &&
            fabs( inY - r->position.y ) < lineHeight / 2 ) {
            // hit

            return r;
            }
        }
    
    return NULL;
    }



// inName NOT destroyed
// result destroyed by caller
static char *convertToLastName( const char *inName ) {
    
    char *convertedName;
            
    int partCount;
    char **parts = split( inName, " ",
                          &partCount );
            
    if( partCount != 3 ) {
        // default to using full name
        convertedName = stringDuplicate( inName );
        }
    else {
        convertedName = stringDuplicate( parts[2] );
        }
    for( int i=0; i<partCount; i++ ) {
        delete [] parts[i];
        }
    delete [] parts;
    
    return convertedName;
    }



void RobPickList::setTip( HouseRecord *inRecord ) {
    char *tip;
        
    if( mRobberyLog ) {
        char *lastName = convertToLastName( inRecord->lastRobberName );
        
        const char *tipKey = "replayPickListTip";
        if( inRecord->bountyFlag ) {
            tipKey = "replayPickListBountyTip";
            }

        tip = autoSprintf( translate( tipKey ), 
                           lastName, inRecord->lootValue, 
                           inRecord->stat.robAttempts, 
                           inRecord->robberDeaths );
        delete [] lastName;
        }
    else {
        // transform into possessive form of last name only
            
        char *lastName = convertToLastName( inRecord->characterName );
            
        char *possessiveName;
            
        // add possessive to end
        if( lastName[ strlen( lastName ) - 1 ] == 's' ) {
            possessiveName = autoSprintf( "%s'", lastName );
            }
        else {
            possessiveName = autoSprintf( "%s's", lastName );
            }
            
        delete [] lastName;
            
            

        tip = autoSprintf( translate( "robPickListTip" ), 
                           inRecord->stat.numSteps, 
                           possessiveName, 
                           inRecord->robberDeaths );
        delete [] possessiveName;
        }
        
    setToolTip( tip );

    delete [] tip;
    }




void RobPickList::pointerUp( float inX, float inY ) {
    HouseRecord *r = getHitRecord( inX, inY );
    
    if( r != NULL ) {
        
        // unselect all
        for( int j=0; j<mHouseList.size(); j++ ) {
            HouseRecord *rB = mHouseList.getElement( j );
            rB->selected = false;
            }
            
        r->selected = true;
        setTip( r );
        mHover = true;
        
        if( ! mRobberyLog ) {
            mIgnoreButton.setVisible( true );
            mClearIgnoreListButton.setVisible( false );
            }
        

        fireActionPerformed( this );
        return;
        }
    else if( mHover ) {
        // just hovered out
        setToolTip( NULL );
        mHover = false;
        }

    }





void RobPickList::pointerMove( float inX, float inY ) {
    HouseRecord *r = getHitRecord( inX, inY );
    
    if( r != NULL ) {
        mHover = true;
        setTip( r );
        }
    else if( mHover ) {
        // just hovered out
        setToolTip( NULL );
        mHover = false;
        }
    }


void RobPickList::pointerDrag( float inX, float inY ) {
    pointerUp( inX, inY );
    }


void RobPickList::pointerDown( float inX, float inY ) {
    pointerUp( inX, inY );
    }




void RobPickList::clearHouseList() {
    for( int i=0; i<mHouseList.size(); i++ ) {
        delete [] mHouseList.getElement(i)->characterName;
        delete [] mHouseList.getElement(i)->rawCharacterName;
        delete [] mHouseList.getElement(i)->lastRobberName;
        }
    mHouseList.deleteAll();
    }

