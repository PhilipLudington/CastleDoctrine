#include "HouseObjectPicker.h"


#include "ticketHash.h"
#include "houseObjects.h"
#include "tools.h"

#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"
#include "minorGems/game/drawUtils.h"

#include "minorGems/util/stringUtils.h"


extern Font *mainFont;

extern char *serverURL;

extern int userID;

extern double frameRateFactor;



// these cannot be manually placed by user
#define BLOCK_LIST_SIZE 3

static const char *blockList[BLOCK_LIST_SIZE] = 
{ "start",
  "vault_goal",
  "wall_exterior" 
    };


                                    

HouseObjectPicker::HouseObjectPicker( double inX, double inY,
                                      char inTools )
        : PageComponent( inX, inY ),
          mHover( false ),
          mDragOver( false ),
          mShowTools( inTools ), 
          mSpriteScale( 1 / 32.0 ),
          mShouldShowGridView( false ),
          mSelectedIndex( -1 ),
          mPixWidth( 1/16.0 ),
          mUpButton( "up.tga", -1.25, 1, mPixWidth ), 
          mDownButton( "down.tga", -1.25, -1, mPixWidth ),
          mGridViewButton( "gridView.tga", 1.25, 1, mPixWidth ),
          // blank in place for pickers that don't use it (like tool picker)
          mWifeName( stringDuplicate( "" ) ) {

    mUpButton.setVisible( false );
    mDownButton.setVisible( false );
    mGridViewButton.setVisible( false );
    
    addComponent( &mUpButton );
    addComponent( &mDownButton );
    addComponent( &mGridViewButton );

    mUpButton.addActionListener( this );
    mDownButton.addActionListener( this );
    mGridViewButton.addActionListener( this );

    const char *gridKey = "gridViewObjects";
    if( mShowTools ) {
        gridKey = "gridViewTools";
        }
    mGridViewButton.setMouseOverTip( gridKey );

    
    int numObjects;
    int *idList;

    if( mShowTools ) {
        idList = getFullToolIDList( &numObjects );
        }
    else {
        idList = getFullObjectIDList( &numObjects );
        }

        
    for( int i=0; i<numObjects; i++ ) {
        int id = idList[i];
        
        const char *name;
        if( mShowTools ) {
            name = getToolName( id );
            }
        else {
            name = getObjectName( id );
            }
        
        char blocked = false;
        for( int b=0; b<BLOCK_LIST_SIZE; b++ ) {
            if( strcmp( name, blockList[b] ) == 0 ) {
                blocked = true;
                break;
                }
            }
        
        if( !blocked ) {
            mLocalPresentIDs.push_back( id );
            }
        }

    delete [] idList;    
    }



HouseObjectPicker::~HouseObjectPicker() {
    if( mWifeName != NULL ) {
        delete [] mWifeName;
        }
    }




void HouseObjectPicker::triggerToolTip() {
    if( mShowTools ) {
        setToolTip( getToolDescription( 
                        mObjectList.getElement( mSelectedIndex )->id ) );
        }
    else {
        char *description = 
            getObjectDescription( 
                mObjectList.getElement( mSelectedIndex )->id, 
                0,
                mWifeName );
        
        setToolTip( description );

        delete [] description;
        }
    }



void HouseObjectPicker::actionPerformed( GUIComponent *inTarget ) {
    char change = false;
    
    if( inTarget == &mUpButton ) {
        mSelectedIndex --;
        mDownButton.setVisible( true );
        if( mSelectedIndex == 0 ) {
            mUpButton.setVisible( false );
            }
        change = true;
        }
    else if( inTarget == &mDownButton ) {
        mSelectedIndex ++;
        mUpButton.setVisible( true );
        if( mSelectedIndex == mObjectList.size() - 1 ) {
            mDownButton.setVisible( false );
            }
        change = true;    
        }
    else if( inTarget == &mGridViewButton ) {
        mShouldShowGridView = true;
        fireActionPerformed( this );
        }
    
    
    if( change ) {
        triggerToolTip();
        
        fireActionPerformed( this );
        }
    
    }


void HouseObjectPicker::setSelectedObject( int inObjectID ) {
    int numIDs = mObjectList.size();
    
    for( int i=0; i<numIDs; i++ ) {
        ObjectPriceRecord *r = mObjectList.getElement( i );
        
        if( r->id == inObjectID ) {
            mSelectedIndex = i;
            // auto-move it to top of stack
            useSelectedObject();

            triggerToolTip();
            fireActionPerformed( this );
            }
        }
    }





char HouseObjectPicker::shouldShowGridView() {
    char show = mShouldShowGridView;
    mShouldShowGridView = false;
    return show;
    }



int HouseObjectPicker::getSelectedObject() {
    
    if( mSelectedIndex >= 0 ) {
        ObjectPriceRecord *r = mObjectList.getElement( mSelectedIndex );
        
        return r->id;
        }
    else {
        return -1;
        }
    }



void HouseObjectPicker::useSelectedObject() {
    ObjectPriceRecord r = *( mObjectList.getElement( mSelectedIndex ) );
        
    // move to top of stack
    
    mObjectList.deleteElement( mSelectedIndex );
    
    mObjectList.push_front( r );
    
    mSelectedIndex = 0;
    
    mUpButton.setVisible( false );
    mDownButton.setVisible( true );
    }



void HouseObjectPicker::step() {
    }



void HouseObjectPicker::draw() {

    if( mSelectedIndex >= 0 ) {
        ObjectPriceRecord *r = mObjectList.getElement( mSelectedIndex );

        int orientation = 0;

        if( !mShowTools ) {    
            int numOrientations = getNumOrientations( r->id, 0 );
            
            if( numOrientations == 4 ) {
                // default to left-facing
                orientation = 3;
                }
            if( numOrientations == 2 ) {
                // default to horizontal
                orientation = 1;
                }
            }
        
        
        
        SpriteHandle sprite;
        SpriteHandle underSprite = NULL;
        SpriteHandle behindSprite = NULL;
        
        if( mShowTools ) {
            sprite = getToolSprite( r->id );
            }
        else {
            sprite = getObjectSprite( r->id, orientation, 0 );
            
            if( isUnderSpritePresent( r->id, 0 ) ) {
                underSprite = getObjectSpriteUnder( r->id, orientation, 0 );
                }
            if( isBehindSpritePresent( r->id, 0 ) ) {
                behindSprite = getObjectSpriteBehind( r->id, orientation, 0 );
                }
            }
        
        doublePair center = { 0, 0 };


        // color like a reactive button if we have only 1 item
        char oneItem = ( mObjectList.size() == 1 );
        if( oneItem && mHover && ! mDragOver ) {    
            setDrawColor( 0.75, 0.75, 0.75, 1 );
            }
        else if( oneItem && mDragOver ) {
            setDrawColor( 0.25, 0.25, 0.25, 1 );
            }
        else {
            setDrawColor( 0.5, 0.5, 0.5, 1 );
            }
        
        drawSquare( center, 1 );
        
        if( mShowTools ) {
            // gray background to match backpack slot backgrounds
            
            if( oneItem && mDragOver ) {
                setDrawColor( 0.1, 0.1, 0.1, 1 );
                }
            else {
                setDrawColor( 0.25, 0.25, 0.25, 1 );
                }
            }
        else {
            // no drag-over darkening behavior
            // (already black)
            setDrawColor( 0, 0, 0, 1 );
            }
        
        drawSquare( center, 1 - mPixWidth );

        if( underSprite != NULL ) {
            // darken a bit
            setDrawColor( 0.75, 0.75, 0.75, 1 );
            drawSprite( underSprite, center, mSpriteScale );
            }

        setDrawColor( 1, 1, 1, 1 );
        
        if( behindSprite != NULL ) {
            drawSprite( behindSprite, center, mSpriteScale );
            }

        drawSprite( sprite, center, mSpriteScale );

        char *priceString = autoSprintf( "$%d", r->price );
        
        doublePair pricePos ={ -0.25, -1.5 };
        
        TextAlignment align = alignLeft;
        
        // no up-down buttons to get in the way
        // center price
        if( mObjectList.size() == 1 ) {
            pricePos.x = center.x;
            align = alignCenter;
            }
        
        mainFont->drawString( priceString, pricePos, align );

        delete [] priceString;        
        }
    }




void HouseObjectPicker::setPriceList( const char *inPriceList ) {
    // parse it
    int numBigParts;
    char **bigParts = split( inPriceList, ":", &numBigParts );
    
    if( numBigParts == 3 ) {
        
        char *listBody = bigParts[1];
        
        int numPairs;
        char **pairs = split( listBody, "#", &numPairs );
        
        ObjectPriceRecord *records = new ObjectPriceRecord[ numPairs ];
        
        for( int i=0; i<numPairs; i++ ) {
            int numParts;
            char **parts = split( pairs[i], "@", &numParts );
            
            if( numParts == 2 ) {
                
                // default in case scan fails
                records[i].id = -1;
                records[i].price = 1;

                sscanf( parts[0], "%d", &( records[i].id ) );

                sscanf( parts[1], "%d", &( records[i].price ) );
                }
            
            for( int p=0; p<numParts; p++ ) {
                delete [] parts[p];
                }
            delete [] parts;

            delete [] pairs[i];
            }
        delete [] pairs;

        setPrices( records, numPairs );
        delete [] records;
        }
    
    for( int i=0; i<numBigParts; i++ ) {
        delete [] bigParts[i];
        }
    delete [] bigParts;
    }



void HouseObjectPicker::setWifeName( const char *inWifeName ) {
    if( mWifeName != NULL ) {
        delete [] mWifeName;
        }
    mWifeName = stringDuplicate( inWifeName );
    }



const char *HouseObjectPicker::getWifeName() {
    return mWifeName;
    }




ObjectPriceRecord *HouseObjectPicker::getPrices( int *outNumPrices ) {
    *outNumPrices = mOriginalObjectList.size();
    return mOriginalObjectList.getElementArray();
    }


        
        
void HouseObjectPicker::setPrices( ObjectPriceRecord *inRecords, 
                                   int inNumRecords ) {

    mObjectList.deleteAll();
    mOriginalObjectList.deleteAll();
    
    for( int i=0; i<inNumRecords; i++ ) {
        ObjectPriceRecord r = inRecords[i];
        
        // only allow server-specified objects that we also have locally
        // (server can specify a subset of our local objects, and we
        //  ignore the rest)

        // this also automatically filters inRecords (the full price list)
        // for either houseObjects or tools, depending on what this picker
        // is displaying (because mLocalPresentIDs will only have one or
        //  the other)
        for( int j=0; j<mLocalPresentIDs.size(); j++ ) {
            if( r.id == *( mLocalPresentIDs.getElement( j ) ) ) {
                mObjectList.push_back( r );
                mOriginalObjectList.push_back( r );
                break;
                }
            }
        }

    mUpButton.setVisible( false );

    if( mObjectList.size() > 1 ) {
        mGridViewButton.setVisible( true );
        mDownButton.setVisible( true );
        }
    else {
        mGridViewButton.setVisible( false );
        mDownButton.setVisible( false );
        }
    

    if( mObjectList.size() > 0 ) {
        mSelectedIndex = 0;
        }
    else {
        mSelectedIndex = -1;
        }
    }



int HouseObjectPicker::getPrice( int inObjectID ) {
    int numIDs = mObjectList.size();
    
    for( int i=0; i<numIDs; i++ ) {
        ObjectPriceRecord *r = mObjectList.getElement( i );
        
        if( r->id == inObjectID ) {
            return r->price;
            }
        }

    return -1;
    }



int HouseObjectPicker::getSellBackPrice( int inObjectID ) {
    int price = getPrice( inObjectID );
    
    if( price == -1 ) {
        return -1;
        }
    
    return price / 2;
    }



char HouseObjectPicker::isInside( float inX, float inY ) {
    return fabs( inX ) < 1 &&
        fabs( inY ) < 1;
    }



void HouseObjectPicker::pointerMove( float inX, float inY ) {
    if( isInside( inX, inY ) ) {
        triggerToolTip();
        mHover = true;
        }
    else {
        if( mHover ) {
            // hover just left
            setToolTip( NULL );
            }
        mHover = false;
        }
    }



void HouseObjectPicker::pointerDrag( float inX, float inY ) {
    if( isInside( inX, inY ) ) {
        mDragOver = true;
        triggerToolTip();
        }
    else {
        if( mDragOver ) {
            // drag just left
            setToolTip( NULL );
            }
        mDragOver = false;
        }
    mHover = false;
    }


void HouseObjectPicker::pointerUp( float inX, float inY ) {
    mDragOver = false;
    if( mObjectList.size() == 1 ) {
        // a single-object picker
        // have it behave like a button
        if( isInside( inX, inY ) ) {
            fireActionPerformed( this );
            mHover = true;
            }
        else {
            mHover = false;
            }
        }
    else if( isInside( inX, inY ) ) {
        // click means show grid view (two ways to show grid view)
        mShouldShowGridView = true;
        fireActionPerformed( this );
        }
    }



void HouseObjectPicker::pointerDown( float inX, float inY ) {
    pointerDrag( inX, inY );
    }



void HouseObjectPicker::setVisible( char inIsVible ) {
    PageComponent::setVisible( inIsVible );
        
    if( ! mVisible ) {
        clearState();
        }
    }


