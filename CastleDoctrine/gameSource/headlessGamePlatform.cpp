#include <stdio.h>


#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SettingsManager.h"
#include "minorGems/network/SocketServer.h"
#include "minorGems/system/Time.h"


#include "minorGems/game/game.h"



#define END_REQUEST "[END_REQUEST]"
#define END_RESPONSE "[END_RESPONSE]"


static int w = 640;
static int h = 480;



static char *readFullRequest( Socket *inSock );




#include "robberySimulator.h"

int main() {
    
    char *customData = getCustomRecordedGameData();

    initDrawString( w, h );
    
    initFrameDrawer( w, h, 60,
                     customData, false );
    
    delete [] customData;
    

    initSimulator();
    
    int port = 
        SettingsManager::getIntSetting( "simulatorServerPort", 5077 );

    char *password = 
        SettingsManager::getStringSetting( "simulatorServerPassword" );
    
    if( password == NULL ) {
        password = stringDuplicate( "secret" );
        }
    

    
    SocketServer server( port, 256 );

    char quit = false;
    
    while( !quit ) {
        printf( "Waiting for connection on port %d, password = '%s'\n", 
                port, password );

        Socket *sock = server.acceptConnection();
        
        if( sock != NULL ) {
            printf( "Got connection\n" );

            char *request = readFullRequest( sock );
            
            if( request != NULL ) {

                char *response = NULL;

                if( strstr( request, "quit" ) == request ) {
                    // starts with quit
                
                    if( strstr( request, password ) != NULL ) {
                        quit = true;
                    
                        response = stringDuplicate( "OK" );
                        }
                    else {
                        response = stringDuplicate( "FAILED" );
                        }
                    }
                else if( strstr( request, "check_alive" ) == request ) {
                    response = stringDuplicate( "OK" );
                    }
                else if( strstr( request, "simulate_robbery" ) == request ) {
                    double startTime = Time::getCurrentTime();
                
                    response = simulateRobbery( request );
                    
                    double netTime = Time::getCurrentTime() - startTime;
                    printf( "Simulation took %f seconds\n", netTime );
                    }
                else {
                    response = stringDuplicate( "FAILED" );
                    }
            

                delete [] request;
            
                sock->send( (unsigned char *)response, strlen( response ), 
                            true, false );
            
                delete [] response;


                sock->send( (unsigned char *)"\n", strlen( "\n" ), 
                            true, false );


                sock->send( (unsigned char *)END_RESPONSE, 
                            strlen( END_RESPONSE ), 
                            true, false );


                sock->sendFlushBeforeClose( 3000 );
                }
            
            delete sock;
            }
        }
    
    delete [] password;
    
    freeSimulator();
    
    freeDrawString();
    freeFrameDrawer();
    }



static char checkForEndMarker( char *inString ) {
    if( strstr( inString, END_REQUEST ) != NULL ) {
        return true;
        }
    return false;
    }




#define READ_SIZE  5000

static char readBuffer[ READ_SIZE + 1 ];


// reads full request up to [END_REQUEST]
// returns result WITHOUT [END_REQUEST] marker at end
// returns NULL on failure
static char *readFullRequest( Socket *inSock ) {
    
    char *readSoFar = stringDuplicate( "" );

    char error = false;
    
    while( !checkForEndMarker( readSoFar ) && ! error ) {
        
        int numRead = inSock->receive( (unsigned char *)readBuffer, 
                                       READ_SIZE, 0 );
    
        if( numRead >= 0 ) {
            readBuffer[ numRead ] = '\0';
            
            char *newReadSoFar = concatonate( readSoFar, readBuffer );
            
            delete [] readSoFar;
            
            readSoFar = newReadSoFar;
            }
        else if( numRead != -2 ) {
            // not timeout, real error
            error = true;
            }
        }
    
    if( error ) {
        return NULL;
        }
    

    char found;
    char *message = replaceOnce( readSoFar, END_REQUEST,
                                 "", &found);

    delete [] readSoFar;

    char *trimmedMessage = trimWhitespace( message );
    
    delete [] message;

    return trimmedMessage;
    }






void loadingFailed( const char *inFailureMessage ) {

    }


unsigned int getRandSeed() {
    return time( NULL );
    }


time_t game_time( time_t *__timer ) {
    return time( __timer );
    }


void mapKey( unsigned char inFromKey, unsigned char inToKey ) {
    }


void toggleKeyMapping( char inMappingOn ) {
    }

char isCommandKeyDown() {
    return false;
    }




char isClipboardSupported() {
    return false;
    }



char *getClipboardText() {
    return stringDuplicate( "" );
    }




int getSampleRate() {
    return 22050;
    }


void setSoundPlaying( char inPlaying ) {
    }




void lockAudio() {
    }


void unlockAudio() {
    }



char isSoundRunning() {
    return false;
    }



static float centerX, centerY;


void setViewCenterPosition( float inX, float inY ) {
    centerX = inX;
    centerY = inY;
    }


doublePair getViewCenterPosition() {
    doublePair result = { centerX, centerY };
    
    return result;
    }



void setViewSize( float inSize ) {
    }



void setCursorVisible( char inIsVisible ) {
    }



void grabInput( char inGrabOn ) {
    }



void setMouseReportingMode( char inWorldCoordinates ) {
    }



void warpMouseToCenter( int *outNewMouseX, int *outNewMouseY ) {
    *outNewMouseX = 0;
    *outNewMouseY = 0;
    }




const char *translate( const char *inTranslationKey ) {
    return inTranslationKey;
    }




// pause and resume the game
void pauseGame() {
    }


char isPaused() {
    return false;
    }



void blockQuitting( char inNoQuitting ) {
    }


char isQuittingBlocked() {
    return false;
    }



void wakeUpPauseFrameRate() {
    }



char isGamePlayingBack() {
    return false;
    }








void saveScreenShot( const char *inPrefix ) {
    }





int startWebRequest( const char *inMethod, const char *inURL,
                     const char *inBody ) {
    return 0;
    }


// take anoter non-blocking step
// return 1 if request complete
// return -1 if request hit an error
// return 0 if request still in-progress
int stepWebRequest( int inHandle ) {
    return 1;
    }



char *getWebResult( int inHandle ) {
    return stringDuplicate( "" );
    }



void clearWebRequest( int inHandle ) {
    }








#include "minorGems/game/gameGraphics.h"





void setDrawColor( float inR, float inG, float inB, float inA ) {
    }


void setDrawFade( float inA ) {
    }



// defaults to non-additive blending
void toggleAdditiveBlend( char inAdditive ) {
    }


// defautls to nearest-neighbor texture magnification
void toggleLinearMagFilter( char inLinearFilterOn ) {
    }


// draw using last set color
// four vertices per quad
void drawQuads( int inNumQuads, double inVertices[] ) {
    }


// four r,g,b,a values per quad vertex
void drawQuads( int inNumQuads, double inVertices[], float inVertexColors[] ) {
    }


// draw using last set color
// three vertices per triangle
void drawTriangles( int inNumTriangles, double inVertices[], 
                    char inStrip, char inFan ) {
    }


// four r,g,b,a values per triangle vertex
void drawTrianglesColor( int inNumTriangles, double inVertices[], 
                         float inVertexColors[],
                         char inStrip, char inFan ) {
    }


// trims drawing operations to a rectangular region
// values in view space
void enableScissor( double inX, double inY, double inWidth, double inHeight ) {
    }

void disableScissor() {
    }



// switch into stencil-buffer drawing mode
// inAdd determines whether drawn primitives add to or subtract from
//   pass-through area of stencil
void startAddingToStencil( char inDrawColorToo, char inAdd ) {
    }

// switch into mode where further drawing operations are passed through
// stenciled area
void startDrawingThroughStencil( char inInvertStencil ) {
    }

// stop stenciling and clear stencil buffer
void stopStencil() {
    }


// stop stenciling without clearing stencil buffer
// (more efficient in cases where the buffer doesn't need to be cleared)
void disableStencil() {
    }




typedef void * SpriteHandle;


static int dummySpriteValue = 1;

static SpriteHandle dummySprite = &dummySpriteValue;


// loads sprite from graphics directory
// can be NULL on load failure
SpriteHandle loadSprite( const char *inTGAFileName, 
                         char inTransparentLowerLeftCorner ) {
    return dummySprite;
    }

// same, but loads from the main directory
SpriteHandle loadSpriteBase( const char *inTGAFileName, 
                             char inTransparentLowerLeftCorner ) {
    return dummySprite;
    }

SpriteHandle fillSprite( Image *inImage, 
                         char inTransparentLowerLeftCorner ) {
    return dummySprite;
    }

SpriteHandle fillSprite( unsigned char *inRGBA, 
                         unsigned int inWidth, unsigned int inHeight ) {
    return dummySprite;
    }

// fill a one-channel (alpha-only) sprite
// other channels will be set to black.
// (more efficient than full RGBA for things like shadows)
SpriteHandle fillSpriteAlphaOnly( unsigned char *inA,
                                  unsigned int inWidth, 
                                  unsigned int inHeight ) {
    return dummySprite;
    }



// reads from the graphics directory
// Image destroyed by caller
Image *readTGAFile( const char *inTGAFileName ) {
    return new Image( 32, 32, 4, true );
    
    }

// same, but reads from the main directory
Image *readTGAFileBase( const char *inTGAFileName ) {
    return new Image( 32, 32, 4, true );
    }


// write a TGA file into main directory
// Image destroyed by caller
void writeTGAFile( const char *inTGAFileName, Image *inImage ) {
    }


void freeSprite( SpriteHandle inSprite ) {
    }


// draw with current draw color
// mag filter defaults to off (nearest neighbor, big pixels)
void drawSprite( SpriteHandle inSprite, doublePair inCenter, 
                 double inZoom ) {
    }


// draw with current draw color, but ignore sprite's colors and use
// only it's alpha.
void drawSpriteAlphaOnly( SpriteHandle inSprite, doublePair inCenter, 
                          double inZoom ) {
    }



// dummy version of this
Image *getScreenRegion( double inX, double inY, 
                        double inWidth, double inHeight ) {
    return new Image( 32, 32, 3, true );
    }


