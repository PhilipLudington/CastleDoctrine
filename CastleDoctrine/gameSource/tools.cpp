// if you're updating tool reaches, you need to set this flag to true
// and run the game once to generate signatures for each reach.txt file.
// After doing this once, you can set the flag back to false and recompile.
// (or leave it at false if you don't want the game to check signatures at
//  all).
// Please don't abuse this in clients that connect to the main server (don't
// give yourself the power to club dogs from across the room, for
// example).
//
// If you change reaches without regenerating signatures,
// loading will fail.
static char regenerateReachSignatures = false;

static const char *reachSignatureKey = "Please don't abuse this key.";



#include "tools.h"


#include "gameElements.h"

#include "houseObjects.h"

#include "minorGems/util/stringUtils.h"
#include "minorGems/game/game.h"




typedef struct toolRecord {
        int id;
        
        char *name;
        
        char *description;
        char *descriptionPlural;
        
        SpriteHandle sprite;

        int reach;

    } toolRecord;



static SimpleVector<toolRecord> tools;

static int idSpaceSize = 0;

// some may be -1
static int *idToIndexMap = NULL;




#include "minorGems/crypto/hashes/sha1.h"

// sha1 digest of ascii base-10 tool id and reach concatonated with the 
// key above
static char *computeReachSignature( toolRecord *inRecord ) {
    char *reachString = autoSprintf( "%d %d %s", 
                                     inRecord->id,
                                     inRecord->reach, reachSignatureKey );
    
    char *sig = computeSHA1Digest( reachString );

    delete [] reachString;
    
    return sig;
    }




void initTools() {
    File elementsDir( NULL, "gameElements" );

    if( !elementsDir.exists() || !elementsDir.isDirectory() ) {
        return;
        }
    
    File *toolsDir = elementsDir.getChildFile( "tools" );
    
    if( toolsDir == NULL ) {
        return;
        }
    else if( !toolsDir->exists() || !toolsDir->isDirectory() ) {
        delete toolsDir;
        return;
        }
    
    int numTools;
    File **toolNameDirs = toolsDir->getChildFiles( &numTools );

    delete toolsDir;
    
    if( toolNameDirs == NULL ) {
        return;
        }
    
    
    for( int i=0; i<numTools; i++ ) {
        
        File *f = toolNameDirs[i];
        

        if( f->exists() && f->isDirectory() ) {
            
            char completeRecord = true;

            toolRecord r;
            
            r.name = f->getFileName();
            r.description = NULL;
            r.descriptionPlural = NULL;
            r.sprite = NULL;
            
            File *infoFile = f->getChildFile( "info.txt" );
            
            completeRecord = readInfoFile( infoFile, 
                                           &( r.id ), &( r.description ) );
            delete infoFile;

            if( completeRecord ) {
                
                File *pluralFile = f->getChildFile( "plural.txt" );
            
                completeRecord = readPluralFile( pluralFile, 
                                                 &( r.descriptionPlural ) );
                delete pluralFile;
                }


            if( completeRecord ) {

                // read reach, if present (if not, default to 1)
                r.reach = 1;

                File *reachFile = f->getChildFile( "reach.txt" );
            
                if( reachFile->exists() ) {
                    
                    char *reach = reachFile->readFileContents();
                
                    sscanf( reach, "%d", &( r.reach ) );

                    delete [] reach;
                    }
                
                delete reachFile;
                

                
                File *reachSigFile = f->getChildFile( "reachSignature.txt" );
                char *reachSigContents = NULL;
                
                if( reachSigFile->exists() ) {
                    reachSigContents = reachSigFile->readFileContents();    
                    }
                delete reachSigFile;
                

                char reachSigOK = true;
    
                if( regenerateReachSignatures ) {
                    // ignore reachSignature.txt and generate a new one
                    char *newSig = computeReachSignature( &r );
                    
                    File *childFile = 
                        f->getChildFile( "reachSignature.txt" );
                    if( childFile != NULL ) {
                        childFile->writeToFile( newSig );
                        delete childFile;
                        }
                    delete [] newSig;
                    }
                else if( reachSigContents == NULL ) {
                    reachSigOK = false;
                    }
                else {
                    // else check it
                    char *sig = trimWhitespace( reachSigContents );
        
                    char *trueSig = computeReachSignature( &r );
        
                    if( strcmp( trueSig, sig ) != 0 ) {
                        reachSigOK = false;
                        }
                    delete [] sig;
                    delete [] trueSig;
                    }
                
                if( reachSigContents != NULL ) {
                    delete [] reachSigContents;
                    }
    
                
                
                if( !reachSigOK ) {
                    char *dirName = f->getFullFileName();
                    char *message = autoSprintf( 
                        "%s\n%s",
                        translate( "badReachSignature" ),
                        dirName );
                    delete [] dirName;
                    
                    loadingFailed( message );
                    delete [] message;
                    }




                
                // look for sprite TGA
                int numChildFiles;
                File **childFiles = f->getChildFiles( &numChildFiles );
    
                char *tgaPath = NULL;
                char *shadeMapTgaPath = NULL;
                
                for( int j=0; j<numChildFiles; j++ ) {
        
                    File *f = childFiles[j];
        
                    char *name = f->getFileName();
                    if( strstr( name, "_shadeMap.tga" ) != NULL ) {
                        if( shadeMapTgaPath != NULL ) {
                            delete [] shadeMapTgaPath;
                            }
                        shadeMapTgaPath = f->getFullFileName();
                        }
                    else if( strstr( name, ".tga" ) != NULL ) {
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
                    // assume only one orientation here
                    // discard extras
                    SpriteHandle readSprites[ MAX_ORIENTATIONS ];
                    
                    int numOrientations = 
                        readShadeMappedSprites( tgaPath, shadeMapTgaPath,
                                                readSprites );
                    
                    if( numOrientations == 0 ) {
                        completeRecord = false;
                        }
                    else {
                        r.sprite = readSprites[0];
                    
                        for( int o=1; o<numOrientations; o++ ) {
                            freeSprite( readSprites[o] );
                            }
                        }
                    }
                else {
                    if( shadeMapTgaPath != NULL ) {
                        delete [] shadeMapTgaPath;
                        }
                    completeRecord = false;
                    }
                }
            
            if( completeRecord ) {
                if( r.id >= idSpaceSize ) {
                    idSpaceSize = r.id + 1;
                    }
                tools.push_back( r );
                }
            else {
                delete [] r.name;
                if( r.description != NULL ) {
                    delete [] r.description;
                    }
                if( r.descriptionPlural != NULL ) {
                    delete [] r.descriptionPlural;
                    }
                if( r.sprite != NULL ) {
                    freeSprite( r.sprite );
                    }
                }
            }
        delete f;
        }

    delete [] toolNameDirs;


    // build map
    idToIndexMap = new int[idSpaceSize];
    for( int i=0; i<idSpaceSize; i++ ) {
        idToIndexMap[i] = -1;
        }

    for( int i=0; i<tools.size(); i++ ) {
        toolRecord r = *( tools.getElement( i ) );
        
        idToIndexMap[r.id] = i;
        }
    }



void freeTools() {
    for( int i=0; i<tools.size(); i++ ) {
        toolRecord r = *( tools.getElement( i ) );
        
        
        delete [] r.name;
        delete [] r.description;
        delete [] r.descriptionPlural;
        
        freeSprite( r.sprite );
        }

    tools.deleteAll();
    
    if( idToIndexMap != NULL ) {
        delete [] idToIndexMap;
        
        idToIndexMap = NULL;
        }
    }



char getToolInRange( int inObjectID ) {
    if( inObjectID < 0 || inObjectID >= idSpaceSize ) {
        return false;
        }

    int index = idToIndexMap[inObjectID];

    if( index == -1 ) {
        return false;
        }
    return true;
    }



static toolRecord *getToolRecord( int inObjectID ) {
    
    if( getToolInRange( inObjectID ) ) {
        return tools.getElement( idToIndexMap[inObjectID] );
        }
    else {
        return tools.getElement( 0 );
        }
    }



SpriteHandle getToolSprite( int inObjectID ) {
    return getToolRecord( inObjectID )->sprite;
    }



int getToolReach( int inObjectID ) {
    return getToolRecord( inObjectID )->reach;
    }



const char *getToolName( int inObjectID ) {
    return getToolRecord( inObjectID )->name;
    }


const char *getToolDescription( int inObjectID  ) {
    return getToolRecord( inObjectID )->description;
    }


const char *getToolDescriptionPlural( int inObjectID  ) {
    return getToolRecord( inObjectID )->descriptionPlural;
    }



int getToolID( const char *inName ) {
    for( int i=0; i<tools.size(); i++ ) {
        toolRecord *r = tools.getElement( i );
        
        if( strcmp( r->name, inName ) == 0 ) {
            return r->id;
            }
        } 

    return -1;
    }



int *getFullToolIDList( int *outNumIDs ) {
    *outNumIDs = tools.size();
    
    int *returnList = new int[ *outNumIDs ];
    
    
    for( int i=0; i<*outNumIDs; i++ ) {
        toolRecord *r = tools.getElement( i );
    
        returnList[i] = r->id;
        }
    
    return returnList;
    }
