#include "GalleryArchive.h"


#include "galleryObjects.h"


#include "minorGems/util/stringUtils.h"




GalleryArchive::GalleryArchive( Font *inDisplayFont, double inX, double inY ) 
        : PageComponent( inX, inY ),
          mSelectedIndex( -1 ),
          mSlot( inDisplayFont, 0, 0, 1/32.0 ),
          mUpButton( "up.tga", -1.3125, .75, 1/16.0 ),
          mDownButton( "down.tga", -1.3125, -.75, 1/16.0 ) {

    addComponent( &mUpButton );
    addComponent( &mDownButton );

    mUpButton.addActionListener( this );
    mDownButton.addActionListener( this );

    addComponent( &mSlot );
    mSlot.addActionListener( this );

    clearObjects();
    }

        

GalleryArchive::~GalleryArchive() {
    }

        
        
void GalleryArchive::actionPerformed( GUIComponent *inTarget ) {
    char change = false;
    
    if( inTarget == &mUpButton ) {
        mSelectedIndex --;
        if( mSelectedIndex < 0 ) {
            mSelectedIndex = mObjectList.size() - 1;
            }
        change = true;
        }
    else if( inTarget == &mDownButton ) {
        mSelectedIndex ++;
        if( mSelectedIndex > mObjectList.size() - 1 ) {
            mSelectedIndex = 0;
            }
        change = true;    
        }
    else if( inTarget == &mSlot ) {
        int id = mSlot.getObject();
        
        if( id != -1 ) {
            fireActionPerformed( this );
            }
        }
    

    
    if( change ) {
        triggerToolTip();

        mSlot.setObject( *( mObjectList.getElement( mSelectedIndex ) ) );
        }
    }

        

void GalleryArchive::clearObjects() {
    mObjectList.deleteAll();
    mSelectedIndex = -1;
    mSlot.setObject( -1 );
    
    mUpButton.setVisible( false );
    mDownButton.setVisible( false );
    }

        

void GalleryArchive::addObject( int inObjectID ) {
    mObjectList.push_back( inObjectID );
    
    mSelectedIndex = mObjectList.size() - 1;
    mSlot.setObject( inObjectID );
    
    if( mObjectList.size() > 1 ) {
        mUpButton.setVisible( true );
        mDownButton.setVisible( true );
        }
    }


        
int GalleryArchive::swapSelectedObject( int inReplacementObjectID ) {
    if( mSelectedIndex != -1 ) {
        
        int *object = mObjectList.getElement( mSelectedIndex );
        
        int oldID = *object;
        
        *object = inReplacementObjectID;
        
        mSlot.setObject( inReplacementObjectID );
        
        triggerToolTip();

        return oldID;
        }
    else {
        return -1;
        }
    }




int GalleryArchive::getObjectCount() {
    return mObjectList.size();
    }



char *GalleryArchive::getContentsString() {
    if( mObjectList.size() == 0 ) {
        return stringDuplicate( "#" );
        }



    SimpleVector<char *> parts;

    // hold selected object and keep it to add at end of list
    // this allows selected object to be remembered server-side
    // (the last object added, from the end of the list, ends up selected
    //  when loaded from the server later)    
    for( int i=0; i<mObjectList.size(); i++ ) {
        
        if( mSelectedIndex != i ) {
            int id = *( mObjectList.getElement( i ) );
            
            parts.push_back( autoSprintf( "%d", id ) );
            }
        }    
    
    if( mSelectedIndex != -1 ) {
        int id = *( mObjectList.getElement( mSelectedIndex ) );
            
        parts.push_back( autoSprintf( "%d", id ) );
        }
    


    char **partsArray = parts.getElementArray();
    
    
    char *fullString = join( partsArray, parts.size(), "#" );

    for( int i=0; i<parts.size(); i++ ) {
        delete [] partsArray[i];
        }
    delete [] partsArray;


    return fullString;
    }



char GalleryArchive::isVisible() {
    if( mObjectList.size() == 0 ) {
        return false;
        }
    return true;
    }



void GalleryArchive::triggerToolTip() {
    
    if( mSelectedIndex != -1 ) {
        setToolTip( 
            getGalleryObjectDescription( 
                *( mObjectList.getElement( mSelectedIndex ) ) ) );
        
        }

    }
