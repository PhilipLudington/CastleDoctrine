#include "galleryObjects.h"


#include "gameElements.h"


#include "houseObjects.h"

#include "minorGems/graphics/filters/FastBlurFilter.h"



typedef struct galleryObjectRecord {
        int id;
        
        char *name;
        
        char *description;
        
        SpriteHandle sprite;

    } galleryObjectRecord;



static SimpleVector<galleryObjectRecord> galleryObjects;

static int idSpaceSize = 0;

// some may be -1
static int *idToIndexMap = NULL;



void initGalleryObjects() {
    File elementsDir( NULL, "gameElements" );

    if( !elementsDir.exists() || !elementsDir.isDirectory() ) {
        return;
        }
    
    File *galleryObjectsDir = elementsDir.getChildFile( "gallery" );
    
    if( galleryObjectsDir == NULL ) {
        return;
        }
    else if( !galleryObjectsDir->exists() || 
             !galleryObjectsDir->isDirectory() ) {
        delete galleryObjectsDir;
        return;
        }
    
    int numGalleryObjects;
    File **galleryObjectNameDirs =
        galleryObjectsDir->getChildFiles( &numGalleryObjects );

    delete galleryObjectsDir;
    
    if( galleryObjectNameDirs == NULL ) {
        return;
        }

    Image *shadow = NULL;
    double *shadowChannel;
    
    
    for( int i=0; i<numGalleryObjects; i++ ) {
        
        File *f = galleryObjectNameDirs[i];
        

        if( f->exists() && f->isDirectory() ) {
            
            char completeRecord = true;

            galleryObjectRecord r;
            
            r.name = f->getFileName();
            r.description = NULL;
            r.sprite = NULL;
            
            File *infoFile = f->getChildFile( "info.txt" );
            
            completeRecord = readInfoFile( infoFile, 
                                           &( r.id ), &( r.description ) );
            delete infoFile;



            if( completeRecord ) {                
                
                // look for sprite TGA
                int numChildFiles;
                File **childFiles = f->getChildFiles( &numChildFiles );
    
                char *tgaPath = NULL;

                for( int j=0; j<numChildFiles; j++ ) {
        
                    File *f = childFiles[j];
        
                    char *name = f->getFileName();
                    if( strstr( name, ".tga" ) != NULL ) {
                        if( tgaPath != NULL ) {
                            delete [] tgaPath;
                            }
                        tgaPath = f->getFullFileName();
                        }
                    
                    delete [] name;

                    delete childFiles[j];
                    }
                delete [] childFiles;


                if( tgaPath != NULL ) {
                    

                    Image *image = readTGAFileBase( tgaPath );
                    delete [] tgaPath;

                    if( image == NULL ) {    
                        completeRecord = false;
                        }
                    else {
                        image = doubleImage( image );
                        
                        int w = image->getWidth();
                        int h = image->getHeight();
                        

                        if( shadow == NULL ) {
                            // shadow not computed yet

                            // make shadow map larger than image
                            // so that box blur filter weird edge behavior
                            // doesn't happen
                            shadow = new Image( w * 2, h * 2, 1, true );
                            
                            shadowChannel = shadow->getChannel( 0 );
                            
                            // white center square, with black bleeding in 
                            // a bit (white square slightly smaller than
                            // target image)
                            for( int y=2; y<h-2; y++ ) {
                                int yFull = y + h/2;
                                for( int x=2; x<w-2; x++ ) {
                                    int xFull = x + w/2;
                                    shadowChannel[ yFull * w * 2 + xFull ] = 1;
                                    }
                                }
                            FastBlurFilter f;
                            
                            shadow->filter( &f );
                            shadow->filter( &f );
                            shadow->filter( &f );
                            shadow->filter( &f );
                            }
                        
                        double *red = image->getChannel( 0 );
                        double *green = image->getChannel( 1 );
                        double *blue = image->getChannel( 2 );
                        
                        for( int y=0; y<h; y++ ) {
                            int yFull = y + h/2;
                            for( int x=0; x<w; x++ ) {
                                int xFull = x + w/2;
                                
                                // mute it's effect a bit, only allowing
                                // it to achieve max of 0.75 darkness
                                double level = 0.25 + 
                                    0.75 *
                                    shadowChannel[ yFull * w * 2 + xFull ];
                                
                                int i = y * w + x;
                                red[i] *= level;
                                green[i] *= level;
                                blue[i] *= level;
                                }
                            }
                        


                        // art has no transparency
                        r.sprite = fillSprite( image, false );
                        delete image;
                        }
                    }
                else {
                    completeRecord = false;
                    }
                }
            
            if( completeRecord ) {
                if( r.id >= idSpaceSize ) {
                    idSpaceSize = r.id + 1;
                    }
                galleryObjects.push_back( r );
                }
            else {
                delete [] r.name;
                if( r.description != NULL ) {
                    delete [] r.description;
                    }
                if( r.sprite != NULL ) {
                    freeSprite( r.sprite );
                    }
                }
            }
        delete f;
        }
    
    if( shadow != NULL ) {
        delete shadow;
        }
    

    delete [] galleryObjectNameDirs;


    // build map
    idToIndexMap = new int[idSpaceSize];
    for( int i=0; i<idSpaceSize; i++ ) {
        idToIndexMap[i] = -1;
        }

    for( int i=0; i<galleryObjects.size(); i++ ) {
        galleryObjectRecord r = *( galleryObjects.getElement( i ) );
        
        idToIndexMap[r.id] = i;
        }
    }



void freeGalleryObjects() {
    for( int i=0; i<galleryObjects.size(); i++ ) {
        galleryObjectRecord r = *( galleryObjects.getElement( i ) );
        
        
        delete [] r.name;
        delete [] r.description;
        
        freeSprite( r.sprite );
        }

    galleryObjects.deleteAll();
    
    if( idToIndexMap != NULL ) {
        delete [] idToIndexMap;
        
        idToIndexMap = NULL;
        }
    }



static galleryObjectRecord *getGalleryObjectRecord( int inObjectID ) {

    int index = idToIndexMap[inObjectID];

    return galleryObjects.getElement( index );
    }



SpriteHandle getGalleryObjectSprite( int inObjectID ) {
    return getGalleryObjectRecord( inObjectID )->sprite;
    }


const char *getGalleryObjectName( int inObjectID ) {
    return getGalleryObjectRecord( inObjectID )->name;
    }


const char *getGalleryObjectDescription( int inObjectID  ) {
    return getGalleryObjectRecord( inObjectID )->description;
    }



int getGalleryObjectID( const char *inName ) {
    for( int i=0; i<galleryObjects.size(); i++ ) {
        galleryObjectRecord *r = galleryObjects.getElement( i );
        
        if( strcmp( r->name, inName ) == 0 ) {
            return r->id;
            }
        } 

    return -1;
    }
