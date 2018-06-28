// if you're updating object properties, you need to set this flag to true
// and run the game once to generate signatures for each properties.txt file.
// After doing this once, you can set the flag back to false and recompile.
// (or leave it at true if you don't want the game to check signatures at
//  all).
// Please don't abuse this in clients that connect to the main server (don't
// give yourself the power to see through walls or see when wires are on, for
// example).
//
// If you change object properties without regenerating signatures,
// loading will fail.
static char regeneratePropertySignatures = false;

static const char *propertySignatureKey = "Please don't abuse this key.";




#include "houseObjects.h"

#include "gameElements.h"

#include "minorGems/game/game.h"

#include "minorGems/util/SimpleVector.h"

#include "minorGems/io/file/File.h"



// redefine F so that it expands each name into a string constant
#undef F
#define F(inName) #inName

const char *propertyIDNames[] = {
	PROPERTY_NAMES
    };



// returns endPropertyID if mapping fails
static propertyID mapNameToID( const char *inName ) {
    for( int i = 0; i < endPropertyID; i++ ) {
        
        if( strcmp( inName, propertyIDNames[i] ) == 0 ) {
            return (propertyID)i;
            }    
        }

    return endPropertyID;
    }



typedef struct houseObjectState {

        // can be 0 if state not present
        int numOrientations;
        
        SpriteHandle stateSprite[ MAX_ORIENTATIONS ];
        
        char behindSpritePresent;

        SpriteHandle stateSpriteBehind[ MAX_ORIENTATIONS ];

        char underSpritePresent;

        SpriteHandle stateSpriteUnder[ MAX_ORIENTATIONS ];
        
        char haloGenerated;
        
        SpriteHandle haloSprite[ MAX_ORIENTATIONS ];

        char properties[ endPropertyID ];

        char *subDescription;

    } houseObjectState;



typedef struct houseObjectRecord {
        int id;
        
        char *name;
        
        char *description;
        
        int numStates;
        
        
        // sprites can be absent for certain entries if state IDs 
        // are sparsely distributed
        // (absent sprites have 0 orientations)
        //
        // MUST have at least 0 defined
        houseObjectState *states;
        

        // ids of other objects that group with this object
        // for tile orientation purposes
        int numGroupWith;
        
        int *groupWith;

        
        // only used temporarily during file parsing
        // names of groupWith objects
        // (we need this because we don't know their IDs until we're
        //  done reading all objects, so we can't fill in groupWith with ids
        //  until after all are read)
        // but these are deleted after the names are converted to ids to
        // fill in groupWith
        char **groupWithNames;


        // true if a property is set for any state of this object
        char propertiesEverSet[ endPropertyID ];
        
    } houseObjectRecord;



static SimpleVector<houseObjectRecord> objects;

static int idSpaceSize = 0;

// some may be -1
static int *idToIndexMap = NULL;



#include "minorGems/crypto/hashes/sha1.h"

// sha1 digest ascii base-10 ID and state number concatonated with
// properties as an ascii bit string of '1' and '0' concatonated
// with the key above
static char *computePropertiesSignature( int inObjectID, int inStateNumber,
                                         houseObjectState *inState ) {
    char propertyString[endPropertyID + 1];
    
    for( int p=0; p<endPropertyID; p++ ) {
        if( inState->properties[p] ) {
            propertyString[p] = '1';
            }
        else {
            propertyString[p] = '0';
            }
        }
    propertyString[ endPropertyID ] = '\0';
    
    char *fullString = autoSprintf( "%d %d %s %s",
                                    inObjectID, inStateNumber, propertyString,
                                    propertySignatureKey );

    char *sig = computeSHA1Digest( fullString );

    delete [] fullString;
    
    return sig;
    }




// doubles image with nearest neighbor interpolation
// destroys inImage
Image *doubleImage( Image *inImage ) {
    int h = inImage->getHeight();
    int w = inImage->getWidth();
    int n = inImage->getNumChannels();
    
    int dH = h * 2;
    int dW = w * 2;
    
    Image *doubledImage = new Image( dW, dH, n, false );    

    double **channels = new double*[n];
    double **dChannels = new double*[n];
    
    
    

    for( int c=0; c<n; c++ ) {
        channels[c] = inImage->getChannel( c );
        dChannels[c] = doubledImage->getChannel( c );
        
        }    
        
        
    for( int dY=0; dY<dH; dY++ ) {
        int y = dY / 2;
        
        for( int dX=0; dX<dW; dX++ ) {
            int x = dX / 2;
            
            int dI = dY * dW + dX;
            
            int i = y * w + x;
            
            for( int c=0; c<n; c++ ) {
                dChannels[c][dI] = channels[c][i];
                }
            }
        }
    
    delete [] channels;
    delete [] dChannels;
    
    delete inImage;

    return doubledImage;
    }


// shading at bottom, on red anchor
// and for forced under shading
static double shadeDark = 0.5;
// steps per double-res pixel
// as we walk up toward yellow anchor
static double shadeStep = 0.035;


static void applyShadeMap( Image *inImage, Image *inShadeMap ) {
    int h = inImage->getHeight();
    int w = inImage->getWidth();
    
    if( inShadeMap->getHeight() != h ||
        inShadeMap->getWidth() != w ) {
        printf( "Failed to apply shade map, dimension mismatch\n" );
        return;
        }
    
    double *r = inImage->getChannel( 0 );
    double *g = inImage->getChannel( 1 );
    double *b = inImage->getChannel( 2 );
    
    double *rMap = inShadeMap->getChannel( 0 );
    double *gMap = inShadeMap->getChannel( 1 );
    double *bMap = inShadeMap->getChannel( 2 );
        
    // index of transparency
    int tI = w * (h-1);
            
    // color of transparency
    double tR = r[tI];
    double tG = g[tI];
    double tB = b[tI];


    // color of shading anchors

    // fixed at pure yellow
    double anchorTopR = 1;
    double anchorTopG = 1;
    double anchorTopB = 0;

    // fixed at pure red
    double anchorBottomR = 1;
    double anchorBottomG = 0;
    double anchorBottomB = 0;

    // fixed at pure green
    double anchorSkipR = 0;
    double anchorSkipG = 1;
    double anchorSkipB = 0;


    // start at bottom of shade map, walking up row by row
    // when we encounter a bottom anchor, we walk up, shading image,
    // until we hit the top anchor.
    // Then we go back to row we were on and continue

    for( int y=h-1; y>=0; y-- ) {        
        for( int x=0; x<w; x++ ) {
            int i = y * w + x;
            
            if( rMap[i] == anchorBottomR &&
                gMap[i] == anchorBottomG &&
                bMap[i] == anchorBottomB ) {
                
                // bottom anchor is 2 pixels tall
                // we're hitting the bottom pixel here
                // clear the top anchor now so that we
                // don't run into it again later
                rMap[i-w] = 0;
                gMap[i-w] = 0;
                bMap[i-w] = 0;
                
                // now walk upward, shading image, until we hit a top anchor
                
                int iUp = i;
                double shadeLevel = shadeDark;
                
                while( iUp >= 0 &&
                       ! ( rMap[iUp] == anchorTopR &&
                           gMap[iUp] == anchorTopG &&
                           bMap[iUp] == anchorTopB ) ) {
                    // not hit top shading anchor yet

                    // skip transparent pixels in image
                    // and pixels flagged with the skip anchor color
                    if( ! ( r[iUp] == tR &&
                            g[iUp] == tG &&
                            b[iUp] == tB ) 
                        &&
                        ! ( rMap[iUp] == anchorSkipR &&
                            gMap[iUp] == anchorSkipG &&
                            bMap[iUp] == anchorSkipB ) ) {
                                                
                        // apply shade level to this image pixel
                        r[iUp] *= shadeLevel;
                        g[iUp] *= shadeLevel;
                        b[iUp] *= shadeLevel;
                        }
                    
                    shadeLevel += shadeStep;

                    if( shadeLevel > 1 ) {
                        shadeLevel = 1;
                        }
                    
                    iUp -= w;
                    }
                
                

                }
            }
        }
    
    }




#include "FastBoxBlurFilter.h"


static SpriteHandle imageToHaloSprite( Image *inImage ) {
    Image *alphaImage = inImage->generateAlphaChannel();
    
    
    int w = alphaImage->getWidth();
    int h = alphaImage->getHeight();

    // room for blur to spread 6 pixels from sprite outline
    int extraRoom = 6;
    
    // extra room
    int haloW = w * 2;
    int haloH = h * 2;
    
    int offsetX = w/2;
    int offsetY = h/2;

    // only apply blur to this region for efficiency
    // (found with profiler)
    int haloRegionStartX = offsetX -  extraRoom - 1;
    int haloRegionStartY = offsetY - extraRoom - 1;

    int haloRegionEndX = offsetX + w + extraRoom + 1;
    int haloRegionEndY = offsetY + h + extraRoom + 1;
    

    int numHaloPixels = haloW * haloH;
    
    double *imageAlpha = alphaImage->getChannel( 3 );

    unsigned char *haloChannel = new unsigned char[ numHaloPixels ];
    
    int *touchedPixels = new int[ numHaloPixels ];
    int numTouched = 0;

    memset( haloChannel, 0, numHaloPixels );
    
    for( int y=0; y<haloH; y++ ) {
        int imY = y - offsetY;
        
        for( int x=0; x<haloW; x++ ) {
            int imX = x - offsetX;
            
            int haloI = y * haloW + x;
            
            int imI = imY * w + imX;
            
            
            if( imY >=0 && imY < h &&
                imX >=0 && imX < w &&
                imageAlpha[ imI] > 0 ) {
                haloChannel[haloI] = 255;
                }
            
            if( y > haloRegionStartY && y < haloRegionEndY 
                && 
                x > haloRegionStartX && x < haloRegionEndX ) {
                // away from border
                touchedPixels[ numTouched ] = haloI;
                numTouched++;
                }
            }
        }
    

    FastBoxBlurFilter blur;
        
        
        
        
    //for( int i=0; i<20; i++ ) {
    for( int i=0; i<1; i++ ) {
        
        blur.applySubRegion( haloChannel,
                             touchedPixels,
                             numTouched,
                             haloW, haloH );
        }

    // now apply threshold 
    // (to create a 1-pixel wide black outline around sprite)
    for( int i=0; i<numHaloPixels; i++ ) {
        if( haloChannel[i] > 0 ) {
            haloChannel[i]=255;
            }
        }

    // blur that outline
    for( int i=0; i<5; i++ ) {
        
        blur.applySubRegion( haloChannel,
                             touchedPixels,
                             numTouched,
                             haloW, haloH );
        }
   
    delete [] touchedPixels;
        

    SpriteHandle haloSprite = fillSpriteAlphaOnly( haloChannel,
                                                   haloW, haloH );
    delete [] haloChannel;
    
    delete alphaImage;


    return haloSprite;
    }




// inTgaPath and inShadeMapTgaPath are deleted if not NULL
// returns number of orientaitons
int readShadeMappedSprites( char *inTgaPath, char *inShadeMapTgaPath,
                            SpriteHandle *inSpriteOrientationArray,
                            SpriteHandle *inHaloSpriteOrientationArray,
                            char inForceUnderShading ) {
    
    Image *image = readTGAFileBase( inTgaPath );
    delete [] inTgaPath;

    if( image == NULL ) {    
        return 0;
        }
    
    image = doubleImage( image );


    if( inShadeMapTgaPath != NULL ) {
       //printf( "Trying to read shade map tga from %s\n", inShadeMapTgaPath );
        
        Image *shadeMapImage = readTGAFileBase( inShadeMapTgaPath );
        delete [] inShadeMapTgaPath;
    
        if( shadeMapImage != NULL ) {
            shadeMapImage = doubleImage( shadeMapImage );
            
            applyShadeMap( image, shadeMapImage );
            
            delete shadeMapImage;
            }
        }
    else if( inForceUnderShading ) {
        double *r = image->getChannel( 0 );
        double *g = image->getChannel( 1 );
        double *b = image->getChannel( 2 );

        int numPixels = image->getHeight() * image->getWidth();
        for( int i=0; i<numPixels; i++ ) {
            r[i] *= shadeDark;
            g[i] *= shadeDark;
            b[i] *= shadeDark;
            }
        }
    
    
    char transCorner = true;

    int fullH = image->getHeight();
    int fullW = image->getWidth();

    int tileH = fullW;

    int numOrientations = fullH / tileH;
    
    //printf( "  Reading %d orientations\n", numOrientations );

    for( int o=0; o<numOrientations; o++ ) {
        
        Image *subImage = image->getSubImage( 0, tileH * o,
                                              fullW, tileH );
        
        inSpriteOrientationArray[o] = fillSprite( subImage, transCorner );
        
        if( inHaloSpriteOrientationArray != NULL ) {
            inHaloSpriteOrientationArray[o] = imageToHaloSprite( subImage );
            }

        delete subImage;
        }

    delete image;
    
    return numOrientations;
    }






static houseObjectState readState( File *inStateDir, int inObjectID,
                                   int inStateNumber ) {
    
    int numChildFiles;
    File **childFiles = inStateDir->getChildFiles( &numChildFiles );
    
    char *tgaPath = NULL;
    char *shadeMapTgaPath = NULL;
    char *behindTgaPath = NULL;
    char *behindShadeMapTgaPath = NULL;
    char *underTgaPath = NULL;
    char *underShadeMapTgaPath = NULL;
    char *propertiesContents = NULL;
    char *propertiesSigContents = NULL;
    char *spriteSigContents = NULL;
    char *subInfoContents = NULL;


    for( int i=0; i<numChildFiles; i++ ) {
        
        File *f = childFiles[i];
        
        char *name = f->getFileName();

        if( strstr( name, "_behind.tga" ) != NULL ) {
            if( behindTgaPath != NULL ) {
                delete [] behindTgaPath;
                }
            behindTgaPath = f->getFullFileName();
            }
        else if( strstr( name, "_behind_shadeMap.tga" ) != NULL ) {
            if( behindShadeMapTgaPath != NULL ) {
                delete [] behindShadeMapTgaPath;
                }
            behindShadeMapTgaPath = f->getFullFileName();
            }
        else if( strstr( name, "_under.tga" ) != NULL ) {
            if( underTgaPath != NULL ) {
                delete [] underTgaPath;
                }
            underTgaPath = f->getFullFileName();
            }
        else if( strstr( name, "_under_shadeMap.tga" ) != NULL ) {
            if( underShadeMapTgaPath != NULL ) {
                delete [] underShadeMapTgaPath;
                }
            underShadeMapTgaPath = f->getFullFileName();
            }
        else if( strstr( name, "_shadeMap.tga" ) != NULL ) {
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
        else if( strcmp( name, "properties.txt" ) == 0 ) {
            if( propertiesContents != NULL ) {
                delete [] propertiesContents;
                }
            propertiesContents = f->readFileContents();
            }
        else if( strcmp( name, "propertiesSignature.txt" ) == 0 ) {
            if( propertiesSigContents != NULL ) {
                delete [] propertiesSigContents;
                }
            propertiesSigContents = f->readFileContents();
            }
        else if( strcmp( name, "spriteSignature.txt" ) == 0 ) {
            if( spriteSigContents != NULL ) {
                delete [] spriteSigContents;
                }
            spriteSigContents = f->readFileContents();
            }
        else if( strcmp( name, "subInfo.txt" ) == 0 ) {
            if( subInfoContents != NULL ) {
                delete [] subInfoContents;
                }
            subInfoContents = f->readFileContents();
            }
        
        delete [] name;

        delete childFiles[i];
        }
    delete [] childFiles;

    
    houseObjectState state;

    state.numOrientations = 0;
    state.behindSpritePresent = false;
    state.underSpritePresent = false;
    state.subDescription = NULL;

    // init property array, all off
    for( int p=0; p<endPropertyID; p++ ) {
        state.properties[p] = false;
        }
    
    // next, read properties.txt file and set flags
    
    if( propertiesContents != NULL ) {
        
        
        SimpleVector<char *> *tokens = 
            tokenizeString( propertiesContents );
        
        
        for( int t=0; t<tokens->size(); t++ ) {
            char *name = *( tokens->getElement(t) );
            
            propertyID p = mapNameToID( name );
            
            if( p != endPropertyID ) {
                state.properties[p] = true;
                }

            delete [] name;    
            }
        delete tokens;


        delete [] propertiesContents;
        }


    char propertiesSigOK = true;
    
    if( regeneratePropertySignatures ) {
        // ignore propertiesSignature.txt and generate a new one
        char *newSig = computePropertiesSignature( inObjectID, inStateNumber,
                                                   &state );
        
        File *childFile = 
            inStateDir->getChildFile( "propertiesSignature.txt" );
        if( childFile != NULL ) {
            childFile->writeToFile( newSig );
            delete childFile;
            }
        delete [] newSig;
        }
    else if( propertiesSigContents == NULL ) {
        propertiesSigOK = false;
        }
    else {
        // else check it
        char *sig = trimWhitespace( propertiesSigContents );
        
        char *trueSig = computePropertiesSignature( inObjectID, inStateNumber,
                                                    &state );
        
        if( strcmp( trueSig, sig ) != 0 ) {
            propertiesSigOK = false;
            }
        delete [] sig;
        delete [] trueSig;
        }
    
    if( propertiesSigContents != NULL ) {
        delete [] propertiesSigContents;
        }
    


    if( !propertiesSigOK ) {
        char *dirName = inStateDir->getFullFileName();
        char *message = autoSprintf( "%s\n%s",
                                     translate( "badPropertiesSignature" ),
                                     dirName );
        delete [] dirName;
        
        loadingFailed( message );
        delete [] message;
        }
    



    if( state.properties[ blockSubInfo ] ) {
        if( subInfoContents != NULL  ) {
            delete [] subInfoContents;
            subInfoContents = NULL;
            }
        }
    
    
    if( subInfoContents != NULL ) {
        char *info = subInfoContents;
        
        // skip the first "
        int readChar = ' ';

        while( readChar != '"' && readChar != '\0' ) {
            readChar = info[0];
            info = &( info[1] );
            }

                
        char *descriptionString = new char[1000];
        // scan a string of up to 999 characters, stopping
        // at the first " character
        int numRead = sscanf( info, "%999[^\"]",
                              descriptionString );
        
        if( numRead == 1 ) {
            state.subDescription = stringDuplicate( descriptionString );
            }

        delete [] descriptionString;
        
        delete [] subInfoContents;
        }


    if( tgaPath == NULL ) {
        if( spriteSigContents != NULL ) {
            delete [] spriteSigContents;
            }
        
        return state;
        }



    if( state.properties[ signedSprite ] ) {
        char sigOK = true;

        File tgaFile( NULL, tgaPath );
        
        int length;
        unsigned char *tgaBytes = tgaFile.readFileContents( &length );

        char *sig = NULL;
        
        if( tgaBytes != NULL ) {
            char *fileHash = computeSHA1Digest( tgaBytes, length );
            
            char *fullString = autoSprintf( "%d %d %s %s",
                                            inObjectID, inStateNumber,
                                            fileHash, propertySignatureKey );
            delete [] fileHash;
            
            sig = computeSHA1Digest( fullString );
            delete [] fullString;

            delete [] tgaBytes;
            }
        
            
        if( regeneratePropertySignatures ) {
            File *childFile = 
                inStateDir->getChildFile( "spriteSignature.txt" );
            if( childFile != NULL ) {
                childFile->writeToFile( sig );
                delete childFile;
                }
            }
        else if( spriteSigContents == NULL ) {
            sigOK = false;
            }
        else {
            char *savedSig = trimWhitespace( spriteSigContents );
        
            if( strcmp( sig, savedSig ) != 0 ) {
                sigOK = false;
                }
            delete [] savedSig;
            }
   

        if( sig != NULL ) {
            delete [] sig;
            }

        if( !sigOK ) {
            char *dirName = inStateDir->getFullFileName();
            char *message = autoSprintf( "%s\n%s",
                                         translate( "badSpriteSignature" ),
                                         dirName );
            delete [] dirName;
            
            loadingFailed( message );
            delete [] message;
            }
        }
    
    if( spriteSigContents != NULL ) {
        delete [] spriteSigContents;
        }

    
    //printf( "Trying to read tga from %s\n", tgaPath );

    // only generate halo sprite if property set
    SpriteHandle *haloArray = NULL;
    state.haloGenerated = false;
    
    if( state.properties[ darkHaloBehind ] ) {
        haloArray = state.haloSprite;
        state.haloGenerated = true;
        }

    state.numOrientations =
        readShadeMappedSprites( tgaPath, shadeMapTgaPath, state.stateSprite,
                                haloArray );
    


    if( behindTgaPath != NULL ) {
        
        //printf( "Trying to read behind-image tga from %s\n", behindTgaPath );

        int numOrientationsPresent = 
            readShadeMappedSprites( behindTgaPath, behindShadeMapTgaPath, 
                                    state.stateSpriteBehind );
        
        if( numOrientationsPresent != state.numOrientations ) {
            printf( "  Orientations (%d) doesn't match "
                    "what is in front TGA (%d)\n",
                    numOrientationsPresent, state.numOrientations );
            return state;
            }
        
        state.behindSpritePresent = true;
        }


    if( underTgaPath != NULL ) {
        
        //printf( "Trying to read under-image tga from %s\n", underTgaPath );

        int numOrientationsPresent = 
            readShadeMappedSprites( underTgaPath, underShadeMapTgaPath, 
                                    state.stateSpriteUnder,
                                    NULL,
                                    // force under shading if no
                                    // shade map present
                                    state.properties[ underLayerShaded ] );
        
        if( numOrientationsPresent != state.numOrientations ) {
            printf( "  Orientations (%d) doesn't match "
                    "what is in front TGA (%d)\n",
                    numOrientationsPresent, state.numOrientations );
            return state;
            }
        
        state.underSpritePresent = true;
        }
    
    

    return state;
    }




void initHouseObjects() {
    File elementsDir( NULL, "gameElements" );

    if( !elementsDir.exists() || !elementsDir.isDirectory() ) {
        return;
        }
    
    File *objectsDir = elementsDir.getChildFile( "houseObjects" );
    
    if( objectsDir == NULL ) {
        return;
        }
    else if( !objectsDir->exists() || !objectsDir->isDirectory() ) {
        delete objectsDir;
        return;
        }
    
    int numObjects;
    File **objectNameDirs = objectsDir->getChildFiles( &numObjects );

    delete objectsDir;
    
    if( objectNameDirs == NULL ) {
        return;
        }
    
    
    for( int i=0; i<numObjects; i++ ) {
        
        File *f = objectNameDirs[i];
        

        if( f->exists() && f->isDirectory() ) {
            
            char completeRecord = true;

            houseObjectRecord r;
            
            r.name = f->getFileName();
            r.description = NULL;

            File *infoFile = f->getChildFile( "info.txt" );
            
            completeRecord = readInfoFile( infoFile, 
                                           &( r.id ), &( r.description ) );
                        
            delete infoFile;


            if( completeRecord ) {

                for( int p=0; p<endPropertyID; p++ ) {
                    r.propertiesEverSet[p] = false;
                    }


                // look for groupWith
                r.numGroupWith = 0;
                r.groupWithNames = NULL;
                r.groupWith = NULL;
                
                File *groupWithFile = f->getChildFile( "groupWith.txt" );
                
                if( groupWithFile->exists() ) {
                    char *groupWithContents = 
                        groupWithFile->readFileContents();
                    
                    SimpleVector<char *> *tokens = 
                        tokenizeString( groupWithContents );
        
                    r.numGroupWith = tokens->size();
                    
                    if( r.numGroupWith > 0 ) {    
                        r.groupWithNames = tokens->getElementArray();
                        }
                    
                    delete tokens;

                    delete [] groupWithContents;
                    }
                delete groupWithFile;

                
                // read states

                int numStateDirs;
                File **stateDirs = f->getChildFiles( &numStateDirs );

                // look for maximum defined state directory
                // then make a sparsely-filled array of states
                // that's big enough to contain that max index number
                int maxStateNumber = 0;

                for( int c=0; c<numStateDirs; c++ ) {
                    if( stateDirs[c]->isDirectory() ) {
                        
                        char *dirName = stateDirs[c]->getFileName();
                        
                        int number;
                        
                        int numRead = sscanf( dirName, "%d", &number );
                        
                        if( numRead == 1 ) {
                            
                            // make sure that dir name is pure state number
                            // skip it, otherwise (i.e., skip:  "test0")

                            char *checkDirName = autoSprintf( "%d", number );
                            
                            if( strcmp( checkDirName, dirName ) == 0 ) {
                                

                                if( maxStateNumber < number ) {
                                    maxStateNumber = number;
                                    }
                                }
                            delete [] checkDirName;
                            }
                        delete [] dirName;
                        }
                    delete stateDirs[c];
                    }
                delete [] stateDirs;
                
                
                r.numStates = maxStateNumber + 1;
                
                r.states = new houseObjectState[ r.numStates ];
                

                for( int s=0; s<r.numStates; s++ ) {
                    // some indexed states might not be present
                    
                    // set numOrientations to zero to mark non-present states
                    r.states[s].numOrientations = 0;
                    r.states[s].subDescription = NULL;
                    
                    char *stateDirName = autoSprintf( "%d", s );
                    
                    File *stateDir = f->getChildFile( stateDirName );
                    
                    
                    if( stateDir->exists() && stateDir->isDirectory() ) {
                        
                        r.states[s] = readState( stateDir, r.id, s );

                        for( int p=0; p<endPropertyID; p++ ) {
                            if( r.states[s].properties[p] ) {
                                r.propertiesEverSet[p] = true;
                                }
                            }
                        }

                    delete stateDir;

                    delete [] stateDirName;                    
                    }
                
                
                if( r.id >= idSpaceSize ) {
                    idSpaceSize = r.id + 1;
                    }

                objects.push_back( r );



                // handle full-object signature for record r
                File *objectSigFile = f->getChildFile( "objectSignature.txt" );
                char *objectSigContents = objectSigFile->readFileContents();
                
                char *savedSig = NULL;
                if( objectSigContents != NULL ) {
                    savedSig = trimWhitespace( objectSigContents );
                    delete[] objectSigContents;
                    }
                
                SimpleVector<char> workingString;
                
                char *headerString = 
                    autoSprintf( "%d %d %d ", r.id, 
                                 r.numStates, r.numGroupWith );
                
                workingString.appendElementString( headerString );
                delete [] headerString;

                for( int i=0; i<r.numStates; i++ ) {
                    char *stateString = 
                        autoSprintf( "%d %d ", 
                                     i, r.states[i].numOrientations );
                    
                    workingString.appendElementString( stateString );
                    delete [] stateString;
                    }
                for( int i=0; i<r.numGroupWith; i++ ) {
                    char *groupWithString = 
                        autoSprintf( "%s ", r.groupWithNames[i] );
                    
                    workingString.appendElementString( groupWithString );
                    delete [] groupWithString;
                    }
                
                char *stringToSign = workingString.getElementString();
                
                char *sig = computeSHA1Digest( stringToSign );
                
                delete [] stringToSign;
                
                char sigOK = true;
            
                if( regeneratePropertySignatures ) {
                    objectSigFile->writeToFile( sig );
                    }
                else if( savedSig == NULL ) {
                    sigOK = false;
                    }
                else if( strcmp( sig, savedSig ) != 0 ) {
                    sigOK = false;
                    }
                
                delete objectSigFile;
               
                if( savedSig != NULL ) {
                    delete [] savedSig;
                    }

                if( sig != NULL ) {
                    delete [] sig;
                    }
                


                if( ! sigOK ) {
                    
                    char *dirName = f->getFullFileName();
                    char *message = 
                        autoSprintf( "%s\n%s",
                                     translate( "badObjectSignature" ),
                                     dirName );
                    delete [] dirName;
                    
                    loadingFailed( message );
                    delete [] message;
                    }

                
                }
            else {
                delete [] r.name;
                if( r.description != NULL ) {
                    delete [] r.description;
                    }
                }
            }

        delete f;
        }

    delete [] objectNameDirs;


    // build map
    idToIndexMap = new int[idSpaceSize];
    for( int i=0; i<idSpaceSize; i++ ) {
        idToIndexMap[i] = -1;
        }

    for( int i=0; i<objects.size(); i++ ) {
        houseObjectRecord r = *( objects.getElement( i ) );
        
        idToIndexMap[r.id] = i;
        }
    

    // now translate groupWith names into IDs (since all objects loaded now)
    for( int i=0; i<objects.size(); i++ ) {
        houseObjectRecord *r = objects.getElement( i );
        
        if( r->numGroupWith > 0 ) {
            r->groupWith = new int[ r->numGroupWith ];

            for( int n=0; n < r->numGroupWith; n++ ) {                
                r->groupWith[ n ] = getObjectID( r->groupWithNames[n] );
                delete [] r->groupWithNames[n];
                }
            delete [] r->groupWithNames;
            r->groupWithNames = NULL;
            }
        }
    

    }




void freeHouseObjects() {
    for( int i=0; i<objects.size(); i++ ) {
        houseObjectRecord r = *( objects.getElement( i ) );
        
        
        delete [] r.name;
        delete [] r.description;
        
        for( int s=0; s<r.numStates; s++ ) {

            for( int o=0; o<r.states[s].numOrientations; o++ ) {
                freeSprite( r.states[s].stateSprite[o] );
                if( r.states[s].behindSpritePresent ) {
                    freeSprite( r.states[s].stateSpriteBehind[o] );
                    }
                if( r.states[s].underSpritePresent ) {
                    freeSprite( r.states[s].stateSpriteUnder[o] );
                    }
                if( r.states[s].haloGenerated ) {
                    freeSprite( r.states[s].haloSprite[o] );
                    }
                }
            
            if( r.states[s].subDescription != NULL ) {
                delete [] r.states[s].subDescription;
                }
            }
        delete [] r.states;

        if( r.groupWith != NULL ) {
            delete [] r.groupWith;
            }
        }

    objects.deleteAll();
    
    if( idToIndexMap != NULL ) {
        delete [] idToIndexMap;
        
        idToIndexMap = NULL;
        }
    }



int *getFullObjectIDList( int *outNumIDs ) {
    *outNumIDs = objects.size();
    
    int *returnList = new int[ *outNumIDs ];
    
    
    for( int i=0; i<*outNumIDs; i++ ) {
        houseObjectRecord *r = objects.getElement( i );
    
        returnList[i] = r->id;
        }
    
    return returnList;
    }




static houseObjectState *getObjectState( int inObjectID, int inState ) {

    int index = idToIndexMap[inObjectID];
    
    houseObjectRecord *r = objects.getElement( index );

    if( inState >= r->numStates ) {
        // switch to default state
        inState = 0;
        }

    houseObjectState *returnState = &( r->states[inState] );
    
    if( returnState->numOrientations == 0 ) {
        // not actually present (sparse array), switch to default

        returnState = &( r->states[0] );
        }
    

    return returnState;
    }




const char *getObjectName( int inObjectID ) {
    houseObjectRecord *r = objects.getElement( idToIndexMap[inObjectID] );
    
    return r->name;
    }



static const char *getObjectDescription( int inObjectID, int inState ) {
    houseObjectState *state = getObjectState( inObjectID, inState );
    
    if( state->subDescription != NULL ) {
        // state-specific description present
        return state->subDescription;
        }
    else {
        // default, universal description
        houseObjectRecord *r = objects.getElement( idToIndexMap[inObjectID] );
        
        return r->description;
        }

    }



char *getObjectDescription( int inObjectID, int inState, 
                            const char *inWifeName ) {
    
    const char *descriptionString = 
        getObjectDescription( inObjectID, inState );
    
    if( isPropertySet( inObjectID, inState, wifeOwns ) ) {
        char *wifeNamePosessive;
        
        if( inWifeName[ strlen(inWifeName) - 1 ] == 's' ) {
            wifeNamePosessive = autoSprintf( "%s'", inWifeName );
            }
        else {
            wifeNamePosessive = autoSprintf( "%s's", inWifeName );
            }
        
        char *returnString = autoSprintf( descriptionString, 
                                          wifeNamePosessive );
        delete [] wifeNamePosessive;
        return returnString;
        }
    else {
        return stringDuplicate( descriptionString );
        }
    }




int getObjectID( const char *inName ) {
    for( int i=0; i<objects.size(); i++ ) {
        houseObjectRecord *r = objects.getElement( i );
        
        if( strcmp( r->name, inName ) == 0 ) {
            return r->id;
            }
        } 

    return -1;
    }







SpriteHandle getObjectSprite( int inObjectID, 
                              int inOrientation, int inState ) {
    
    houseObjectState *state = getObjectState( inObjectID, inState );

    if( inOrientation >= state->numOrientations ) {
        // default
        inOrientation = 0;
        }    

    return state->stateSprite[inOrientation];
    }


SpriteHandle getObjectHaloSprite( int inObjectID, 
                                  int inOrientation, int inState ) {
    
    houseObjectState *state = getObjectState( inObjectID, inState );

    if( inOrientation >= state->numOrientations ) {
        // default
        inOrientation = 0;
        }    

    return state->haloSprite[inOrientation];
    }



SpriteHandle getObjectSpriteBehind( int inObjectID, 
                                    int inOrientation, int inState ) {
    
    houseObjectState *state = getObjectState( inObjectID, inState );

    if( inOrientation >= state->numOrientations ) {
        // default
        inOrientation = 0;
        }    

    return state->stateSpriteBehind[inOrientation];
    }



SpriteHandle getObjectSpriteUnder( int inObjectID, 
                                   int inOrientation, int inState ) {
    
    houseObjectState *state = getObjectState( inObjectID, inState );

    if( inOrientation >= state->numOrientations ) {
        // default
        inOrientation = 0;
        }    

    return state->stateSpriteUnder[inOrientation];
    }




int getNumOrientations( int inObjectID, int inState ) {
    houseObjectState *state = getObjectState( inObjectID, inState );

    return state->numOrientations;
    }



char isBehindSpritePresent( int inObjectID, int inState ) {
    houseObjectState *state = getObjectState( inObjectID, inState );

    return state->behindSpritePresent;
    }


char isUnderSpritePresent( int inObjectID, int inState ) {
    houseObjectState *state = getObjectState( inObjectID, inState );

    return state->underSpritePresent;
    }



char isPropertySet( int inObjectID, int inState, propertyID inProperty ) {
    houseObjectState *state = getObjectState( inObjectID, inState );

    
    return state->properties[ inProperty ];
    }



char isPropertyEverSet( int inObjectID, propertyID inProperty ) {
    int index = idToIndexMap[inObjectID];
    
    houseObjectRecord *r = objects.getElement( index );

    return r->propertiesEverSet[ inProperty ];
    }






char isInGroup( int inObjectID, int inOtherObjectID ) {
    if( inObjectID == inOtherObjectID ) {
        // in same group as self
        return true;
        }

    houseObjectRecord *r = objects.getElement( idToIndexMap[inObjectID] );
    
    if( r->numGroupWith == 0 ) {
        // empty
        return false;
        }

    for( int i=0; i< r->numGroupWith; i++ ) {
        if( r->groupWith[i] == inOtherObjectID ) {
            return true;
            }
        }
    
    return false;
    }

    


