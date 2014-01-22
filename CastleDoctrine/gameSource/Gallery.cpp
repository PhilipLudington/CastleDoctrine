#include "Gallery.h"

#include "message.h"


#include "minorGems/util/stringUtils.h"
#include "minorGems/game/drawUtils.h"



extern double frameRateFactor;


// for an archive that only has 1 item (don't mark it specially as an archive
// or offset it more)
static doublePair lightAcrchivePosition = { 0, -2.5 };

// for an archive that has more than 1 item (leave room for archive heading).
static doublePair heavyAcrchivePosition = { 0, -3 };



Gallery::Gallery( Font *inDisplayFont, double inX, double inY )
        : PageComponent( inX, inY ), 
          mGalleryArchive( inDisplayFont, 0, -3 ),
          mAllowEdit( true ),
          mFade( 1 ),
          mShouldFade( false ) {


    doublePair slotCenter = { 0, 5 };
    
    for( int i=0; i<NUM_GALLERY_SLOTS; i++ ) {
        mGallerySlots[i] = 
            new GallerySlotButton( inDisplayFont,
                                   slotCenter.x, slotCenter.y,
                                   1 / 32.0 );
        
        addComponent( mGallerySlots[i] );
        mGallerySlots[i]->addActionListener( this );
        mGallerySlots[i]->setVisible( false );

        slotCenter.y -= 2.5;
        }
    
    addComponent( &mGalleryArchive );
    mGalleryArchive.addActionListener( this );
    }

        

Gallery::~Gallery() {
    for( int i=0; i<NUM_GALLERY_SLOTS; i++ ) {
        delete mGallerySlots[i];
        }
    }



void Gallery::setAllowEdit( char inAllow ) {
    mAllowEdit = inAllow;
    }



void Gallery::setGalleryContents( const char *inGalleryContents ) {
    
    // clear all
    for( int i=0; i<NUM_GALLERY_SLOTS; i++ ) {
        mGallerySlots[i]->setObject( -1 );
        mGallerySlots[i]->setVisible( false );
        }
    mGalleryArchive.clearObjects();
    

    if( strcmp( inGalleryContents, "#" ) != 0 ) {
        // non-empty
        
        int numParts;
        char **parts = split( inGalleryContents, "#", &numParts );

        for( int j=0; j<numParts; j++ ) {
            int id;
            sscanf( parts[j], "%d", &id );
                
            if( j < NUM_GALLERY_SLOTS ) {    
                mGallerySlots[j]->setObject( id );
                mGallerySlots[j]->setVisible( true );
                }
            else {
                mGalleryArchive.addObject( id );

                if( mGalleryArchive.getObjectCount() > 1 ) {
                    mGalleryArchive.setPosition( heavyAcrchivePosition.x,
                                                 heavyAcrchivePosition.y );
                    }
                else {
                    mGalleryArchive.setPosition( lightAcrchivePosition.x,
                                                 lightAcrchivePosition.y );
                    }
                }
            
            delete [] parts[j];
            }
        delete [] parts;

        }
    }


char *Gallery::getGalleryContents() {
    SimpleVector<char *> parts;
    
    for( int i=0; i<NUM_GALLERY_SLOTS; i++ ) {
        int id = mGallerySlots[i]->getObject();
        
        if( id != -1 ) {
            parts.push_back( autoSprintf( "%d", id ) );
            }
        }

    if( parts.size() == 0 ) {
        // know that archive is empty too
        return stringDuplicate( "#" );
        }
    
    

    char **partsArray = parts.getElementArray();
    
    
    char *fullString = join( partsArray, parts.size(), "#" );

    for( int i=0; i<parts.size(); i++ ) {
        delete [] partsArray[i];
        }
    delete [] partsArray;


    char *archiveString = mGalleryArchive.getContentsString();
    
    if( strcmp( archiveString, "#" ) == 0 ) {
        delete [] archiveString;
        
        return fullString;
        }
    else {
        char *finalString = autoSprintf( "%s#%s", fullString, archiveString );
        
        delete [] fullString;
        delete [] archiveString;
        
        return finalString;
        }
    }



void Gallery::draw() {

    if( mShouldFade && mFade > 0 ) {
        mFade -= 0.02 * frameRateFactor;
        
        if( mFade < 0 ) {
            mFade = 0;
            }
        }
    else if( !mShouldFade && mFade < 1 ) {
        mFade += 0.02 * frameRateFactor;
        
        if( mFade > 1 ) {
            mFade = 1;
            }
        }
    
        
    

    if( mGallerySlots[0]->isVisible() ) {
        
        doublePair labelPos = mGallerySlots[0]->getCenter();
        labelPos.y += 1.5;
        
        drawMessage( "galleryLabel", labelPos, false );
        }
    
    
    if( mGalleryArchive.isVisible() && mGalleryArchive.getObjectCount() > 1 ) {
        
        doublePair labelPos = mGalleryArchive.getCenter();
        labelPos.y += 1.5;
        
        drawMessage( "galleryArchiveLabel", labelPos, false );
        }

    if( mFade < 1 ) {
        
        setDrawColor( 0, 0, 0, 1 - mFade );
    
        drawRect( -2, -5, 3, 10 );
        }
    }


        
void Gallery::actionPerformed( GUIComponent *inTarget ) {
    if( !mAllowEdit ) {
        return;
        }
    
    char hit = false;

    for( int i=0; i<NUM_GALLERY_SLOTS; i++ ) {
        if( inTarget == mGallerySlots[i] ) {
            int thisID = mGallerySlots[i]->getObject();
                
            if( thisID != -1 && i != 0 ) {
                // swap with next higher slot
                int topID = mGallerySlots[i-1]->getObject();
                mGallerySlots[i-1]->setObject( thisID );
                mGallerySlots[i]->setObject( topID );
                }
            hit = true;
            break;
            }
        }

    if( !hit ) {
            
        if( inTarget == &mGalleryArchive ) {
                
            int pulledObject =
                mGalleryArchive.swapSelectedObject( 
                    mGallerySlots[ NUM_GALLERY_SLOTS - 1 ]->getObject() );
                
            mGallerySlots[ NUM_GALLERY_SLOTS - 1 ]->setObject( 
                pulledObject );

            hit = true;
            }
        }
    }



void Gallery::fadeOut( char inShouldFade ) {
    mShouldFade = inShouldFade;
    }


void Gallery::instantFadeOut( char inShouldFade ) {
    mShouldFade = inShouldFade;
    if( mShouldFade ) {
        mFade = 0;
        }
    else {
        mFade = 1;
        }
    }


char Gallery::isVisible() {
    char noneVisible = true;
    
    for( int i=0; i<NUM_GALLERY_SLOTS; i++ ) {
        if( mGallerySlots[i]->isVisible() ) {
            noneVisible = false;
            break;
            }
        }
    
    if( mGalleryArchive.isVisible() ) {
        noneVisible = false;
        }

    if( noneVisible ) {
        return false;
        }
    

    // if slots are visible, but gallery faded out, still counts as not visible

    if( mFade == 0 ) {
        // faded out, and not about to fade in
        return ! mShouldFade;
        }
    
    return true;
    }


