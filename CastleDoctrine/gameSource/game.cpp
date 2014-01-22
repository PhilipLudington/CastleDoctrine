int versionNumber = 30;



#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

//#define USE_MALLINFO

#ifdef USE_MALLINFO
#include <malloc.h>
#endif


#include "minorGems/graphics/Color.h"

#include "minorGems/sound/filters/ReverbSoundFilter.h"




#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/random/CustomRandomSource.h"


// static seed
CustomRandomSource randSource( 34957197 );



#include "minorGems/util/log/AppLog.h"

#include "minorGems/system/Time.h"


#include "minorGems/game/game.h"
#include "minorGems/game/gameGraphics.h"
#include "minorGems/game/Font.h"
#include "minorGems/game/drawUtils.h"




#include "LoginPage.h"
#include "FinalMessagePage.h"
#include "CheckoutHousePage.h"
#include "PaymentReportPage.h"
#include "EditHousePage.h"
#include "LoadBackpackPage.h"
#include "PickerGridPage.h"
#include "AuctionPage.h"
#include "BuyAuctionPage.h"
#include "CheckinHousePage.h"
#include "MenuPage.h"
#include "RobCheckoutHousePage.h"
#include "RobHousePage.h"
#include "RobCheckinHousePage.h"
#include "RobberyReplayMenuPage.h"
#include "FetchRobberyReplayPage.h"
#include "FetchSelfTestReplayPage.h"
#include "ReplayRobHousePage.h"
#include "StaleHousePage.h"
#include "FetchBlueprintPage.h"
#include "ViewBlueprintPage.h"


#include "houseObjects.h"
#include "houseTransitions.h"
#include "tools.h"
#include "galleryObjects.h"


#include "musicPlayer.h"

#include "seededMusic.h"

#include "serialWebRequests.h"

#include "nameProcessing.h"

#include "secureString.h"



GamePage *currentGamePage = NULL;


LoginPage *loginPage;
FinalMessagePage *finalMessagePage;
CheckoutHousePage *checkoutHousePage;
PaymentReportPage *paymentReportPage;
EditHousePage *editHousePage;
LoadBackpackPage *loadBackpackPage;
PickerGridPage *objectPickerGridPage;
PickerGridPage *toolPickerGridPage;
AuctionPage *auctionPage;
BuyAuctionPage *buyAuctionPage;
RobHousePage *selfHouseTestPage;
CheckinHousePage *checkinHousePage;
MenuPage *menuPage;
RobCheckoutHousePage *robCheckoutHousePage;
RobHousePage *robHousePage;
RobCheckinHousePage *robCheckinHousePage;
RobberyReplayMenuPage *robberyReplayMenuPage;
FetchRobberyReplayPage *fetchRobberyReplayPage;
FetchSelfTestReplayPage *fetchSelfTestReplayPage;
ReplayRobHousePage *replayRobHousePage;
StaleHousePage *staleHousePage;
StaleHousePage *staleHouseDeadPage;
FetchBlueprintPage *fetchBlueprintPage;
ViewBlueprintPage *viewBlueprintPage;
ViewBlueprintPage *viewBlueprintDuringRobPage;


// position of view in world
doublePair lastScreenViewCenter = {0, 0 };



// world with of one view
double viewWidth = 20;

// fraction of viewWidth visible vertically (aspect ratio)
double viewHeightFraction;

int screenW, screenH;

char initDone = false;

float mouseSpeed;

int musicOff;
int diffHighlightsOff;

int webRetrySeconds;


double frameRateFactor = 1;


char firstDrawFrameCalled = false;


char upKey = 'w';
char leftKey = 'a';
char downKey = 's';
char rightKey = 'd';




const char *getWindowTitle() {
    return "The Castle Doctrine";
    }


const char *getAppName() {
    return "CastleDoctrine";
    }


const char *getFontTGAFileName() {
    return "font_8_16.tga";
    }


char isDemoMode() {
    return false;
    }


const char *getDemoCodeSharedSecret() {
    return "fundamental_right";
    }


const char *getDemoCodeServerURL() {
    return "http://FIXME/demoServer/server.php";
    }



char gamePlayingBack = false;


Font *mainFont;
Font *mainFontFixed;
Font *numbersFontFixed;

Font *tinyFont;


char *reflectorURL = NULL;
char *serverURL = NULL;


char *userEmail = NULL;
int userID = -1;
char *downloadCode = NULL;
// each new request to server must use next sequence number
int serverSequenceNumber = -1;

int playerIsAdmin = 0;



static char wasPaused = false;
static float pauseScreenFade = 0;

static char *currentUserTypedMessage = NULL;



// for delete key repeat during message typing
static int holdDeleteKeySteps = -1;
static int stepsBetweenDeleteRepeat;





#define SETTINGS_HASH_SALT "another_loss"


static const char *customDataFormatWriteString = 
    "version%d_mouseSpeed%f_musicOff%d_"
    "diffHighlightsOff%d_webRetrySeconds%d_downloadCode%s_email%s";

static const char *customDataFormatReadString = 
    "version%d_mouseSpeed%f_musicOff%d_"
    "diffHighlightsOff%d_webRetrySeconds%d_downloadCode%10s_email%99s";


char *getCustomRecordedGameData() {    
    
    float mouseSpeedSetting = 
        SettingsManager::getFloatSetting( "mouseSpeed", 1.0f );
    int musicOffSetting = 
        SettingsManager::getIntSetting( "musicOff", 0 );
    int diffHighlightsOffSetting = 
        SettingsManager::getIntSetting( "diffHighlightsOff", 0 );
    int webRetrySecondsSetting = 
        SettingsManager::getIntSetting( "webRetrySeconds", 10 );
    
    char *email =
        SettingsManager::getStringSetting( "email" );
    if( email == NULL ) {
        email = stringDuplicate( "*" );
        }
    else {
        // put bogus email in recording files, since we don't
        // need a email during playback anyway (not communicating with 
        // server during playback)
        
        // want people to be able to share playback files freely without
        // divulging their emails
        delete [] email;

        email = stringDuplicate( "redacted@redacted.com" );
        }
    
    
    char *code = SettingsManager::getStringSetting( "downloadCode" );
    
    if( code == NULL ) {
        code = stringDuplicate( "**********" );
        }
    else {
        // put bogus code in recording files, since we don't
        // need a valid code during playback anyway (not communicating with 
        // server during playback)
        
        // want people to be able to share playback files freely without
        // divulging their download codes
        delete [] code;

        code = stringDuplicate( "EMPTYDEEPS" );
        }
    


    char * result = autoSprintf(
        customDataFormatWriteString,
        versionNumber, mouseSpeedSetting, musicOffSetting,
        diffHighlightsOffSetting, webRetrySecondsSetting, code, email );

    delete [] email;
    delete [] code;
    

    return result;
    }



char showMouseDuringPlayback() {
    // since we rely on the system mouse pointer during the game (and don't
    // draw our own pointer), we need to see the recorded pointer position
    // to make sense of game playback
    return true;
    }



char *getHashSalt() {
    return stringDuplicate( SETTINGS_HASH_SALT );
    }




void initDrawString( int inWidth, int inHeight ) {
    mainFont = new Font( getFontTGAFileName(), 1, 4, false );

    setViewCenterPosition( lastScreenViewCenter.x, lastScreenViewCenter.y );

    viewHeightFraction = inHeight / (double)inWidth;

    // monitors vary in width relative to height
    // keep visible vertical view span constant (15)
    // which is what it would be for a view width of 20 at a 4:3 aspect
    // ratio
    viewWidth = 15 * 1.0 / viewHeightFraction;
    
    
    setViewSize( viewWidth );
    }


void freeDrawString() {
    delete mainFont;
    }



void initFrameDrawer( int inWidth, int inHeight, int inTargetFrameRate,
                      const char *inCustomRecordedGameData,
                      char inPlayingBack ) {


    initMusicPlayer();


    // test notes
    lockAudio();
    int sampleRate = getSampleRate();
    //addMusicFilter( new ReverbSoundFilter( 2343, 0.35, sampleRate ) );
    //addMusicFilter( new ReverbSoundFilter( 5543, 0.35, sampleRate ) );
    //addMusicFilter( new ReverbSoundFilter( 6527, 0.35, sampleRate ) );
    //addMusicFilter( new ReverbSoundFilter( 7531, 0.35, sampleRate ) );
    
    addMusicFilter( new ReverbSoundFilter( 18249, 0.5, sampleRate ) );
    //addMusicFilter( new ReverbSoundFilter( 33000, 0.35, sampleRate ) );


    unlockAudio();

    




    gamePlayingBack = inPlayingBack;
    
    screenW = inWidth;
    screenH = inHeight;
    
    if( inTargetFrameRate != 60 ) {
        frameRateFactor = (double)60 / (double)inTargetFrameRate;
        }
    
    TextField::setDeleteRepeatDelays( (int)( 30 / frameRateFactor ),
                                      (int)( 2 / frameRateFactor ) );
    


    setViewCenterPosition( lastScreenViewCenter.x, lastScreenViewCenter.y );

    viewHeightFraction = inHeight / (double)inWidth;

    // monitors vary in width relative to height
    // keep visible vertical view span constant (15)
    // which is what it would be for a view width of 20 at a 4:3 aspect
    // ratio
    viewWidth = 15 * 1.0 / viewHeightFraction;
    
    
    setViewSize( viewWidth );


    
    

    

    setCursorVisible( true );
    grabInput( false );
    
    // world coordinates
    setMouseReportingMode( true );
    
    
    
    
    mainFontFixed = new Font( getFontTGAFileName(), 1, 4, true );
    numbersFontFixed = new Font( getFontTGAFileName(), 1, 4, true, 1.0, 5 );
    
    tinyFont = new Font( "font_4_8.tga", 1, 2, false );


    float mouseSpeedSetting = 1.0f;
    
    int musicOffSetting = 0;
    int diffHighlightsOffSetting = 0;
    int webRetrySecondsSetting = 10;

    userEmail = new char[100];
    downloadCode = new char[11];
    
    int readVersionNumber;
    
    int numRead = sscanf( inCustomRecordedGameData, 
                          customDataFormatReadString, 
                          &readVersionNumber,
                          &mouseSpeedSetting, 
                          &musicOffSetting,
                          &diffHighlightsOffSetting,
                          &webRetrySecondsSetting,
                          downloadCode,
                          userEmail );
    if( numRead != 7 ) {
        // no recorded game?
        }
    else {

        if( readVersionNumber != versionNumber ) {
            AppLog::printOutNextMessage();
            AppLog::warningF( 
                "WARNING:  version number in playback file is %d "
                "but game version is %d...",
                readVersionNumber, versionNumber );
            }

        if( strcmp( downloadCode, "**********" ) == 0 ) {
            delete [] downloadCode;
            downloadCode = NULL;
            }
        if( strcmp( userEmail, "*" ) == 0 ) {
            delete [] userEmail;
            userEmail = NULL;
            }
        }
    
    if( !inPlayingBack ) {
        // read REAL email and download code from settings file

        delete [] userEmail;
        
        userEmail = SettingsManager::getStringSetting( "email" );    

        
        delete [] downloadCode;
        
        downloadCode = SettingsManager::getStringSetting( "downloadCode" );    
        }
    

    
    double mouseParam = 0.000976562;

    mouseParam *= mouseSpeedSetting;

    mouseSpeed = mouseParam * inWidth / viewWidth;

    musicOff = musicOffSetting;
    diffHighlightsOff = diffHighlightsOffSetting;
    webRetrySeconds = webRetrySecondsSetting;

    reflectorURL = SettingsManager::getStringSetting( "reflectorURL" );

    if( reflectorURL == NULL ) {
        reflectorURL = 
            stringDuplicate( 
                "http://localhost/jcr13/castleReflector/server.php" );
        }


    initHouseObjects();    
    initTools();
    initGalleryObjects();
    
    initHouseTransitions();
    

    loginPage = new LoginPage();
    finalMessagePage = new FinalMessagePage();
    checkoutHousePage = new CheckoutHousePage();
    paymentReportPage = new PaymentReportPage();
    editHousePage = new EditHousePage();
    loadBackpackPage = new LoadBackpackPage();
    objectPickerGridPage = new PickerGridPage( false );
    toolPickerGridPage = new PickerGridPage( true );
    auctionPage = new AuctionPage();
    buyAuctionPage = new BuyAuctionPage();
    
    selfHouseTestPage = new RobHousePage();
    selfHouseTestPage->showBackpack( false );
    
    checkinHousePage = new CheckinHousePage();
    menuPage = new MenuPage();
    robCheckoutHousePage = new RobCheckoutHousePage();
    robHousePage = new RobHousePage();
    robCheckinHousePage = new RobCheckinHousePage();
    robberyReplayMenuPage = new RobberyReplayMenuPage();
    fetchRobberyReplayPage = new FetchRobberyReplayPage();
    fetchSelfTestReplayPage = new FetchSelfTestReplayPage();
    replayRobHousePage = new ReplayRobHousePage();
    staleHousePage = new StaleHousePage( false );
    staleHouseDeadPage = new StaleHousePage( true );
    fetchBlueprintPage = new FetchBlueprintPage();
    viewBlueprintPage = new ViewBlueprintPage();
    viewBlueprintDuringRobPage = new ViewBlueprintPage();

    viewBlueprintDuringRobPage->setLive( true );

    currentGamePage = loginPage;

    currentGamePage->base_makeActive( true );



    if( musicOff ) {
        setMusicLoudness( 0 );
        }
    else {
        setMusicLoudness( 1.0 );
        }
    setSoundPlaying( true );



    initDone = true;
    }


void freeFrameDrawer() {
    delete mainFontFixed;
    delete numbersFontFixed;

    delete tinyFont;
    
    if( currentUserTypedMessage != NULL ) {
        delete [] currentUserTypedMessage;
        currentUserTypedMessage = NULL;
        }


    currentGamePage = NULL;
    delete loginPage;
    delete finalMessagePage;
    delete checkoutHousePage;
    delete paymentReportPage;
    delete editHousePage;
    delete loadBackpackPage;
    delete objectPickerGridPage;
    delete toolPickerGridPage;
    delete auctionPage;
    delete buyAuctionPage;
    delete selfHouseTestPage;
    delete checkinHousePage;
    delete menuPage;
    delete robCheckoutHousePage;
    delete robHousePage;
    delete robCheckinHousePage;
    delete robberyReplayMenuPage;
    delete fetchRobberyReplayPage;
    delete fetchSelfTestReplayPage;
    delete replayRobHousePage;
    delete staleHousePage;
    delete staleHouseDeadPage;
    delete fetchBlueprintPage;
    delete viewBlueprintPage;
    delete viewBlueprintDuringRobPage;
    

    freeHouseObjects();
    freeHouseTransitions();
    
    freeTools();
    freeGalleryObjects();

    if( serverURL != NULL ) {
        delete [] serverURL;
        serverURL = NULL;
        }
    if( reflectorURL != NULL ) {
        delete [] reflectorURL;
        reflectorURL = NULL;
        }

    if( downloadCode != NULL ) {
        delete [] downloadCode;
        downloadCode = NULL;
        }
    
    if( userEmail != NULL ) {
        delete [] userEmail;
        userEmail = NULL;
        }
    
    freeMusicPlayer();
    }





    


// draw code separated from updates
// some updates are still embedded in draw code, so pass a switch to 
// turn them off
static void drawFrameNoUpdate( char inUpdate );




static void drawPauseScreen() {

    double viewHeight = viewHeightFraction * viewWidth;

    setDrawColor( 1, 1, 1, 0.5 * pauseScreenFade );
        
    drawSquare( lastScreenViewCenter, 1.05 * ( viewHeight / 3 ) );
        

    setDrawColor( 0.2, 0.2, 0.2, 0.85 * pauseScreenFade  );
        
    drawSquare( lastScreenViewCenter, viewHeight / 3 );
        

    setDrawColor( 1, 1, 1, pauseScreenFade );

    doublePair messagePos = lastScreenViewCenter;

    messagePos.y += 4.5;

    mainFont->drawString( translate( "pauseMessage1" ), 
                           messagePos, alignCenter );
        
    messagePos.y -= 1.25 * (viewHeight / 15);
    mainFont->drawString( translate( "pauseMessage2" ), 
                           messagePos, alignCenter );

    if( currentUserTypedMessage != NULL ) {
            
        messagePos.y -= 1.25 * (viewHeight / 15);
            
        double maxWidth = 0.95 * ( viewHeight / 1.5 );
            
        int maxLines = 9;

        SimpleVector<char *> *tokens = 
            tokenizeString( currentUserTypedMessage );


        // collect all lines before drawing them
        SimpleVector<char *> lines;
        
            
        while( tokens->size() > 0 ) {

            // build up a a line

            // always take at least first token, even if it is too long
            char *currentLineString = 
                stringDuplicate( *( tokens->getElement( 0 ) ) );
                
            delete [] *( tokens->getElement( 0 ) );
            tokens->deleteElement( 0 );
            
            

            

            
            char nextTokenIsFileSeparator = false;
                
            char *nextLongerString = NULL;
                
            if( tokens->size() > 0 ) {

                char *nextToken = *( tokens->getElement( 0 ) );
                
                if( nextToken[0] == 28 ) {
                    nextTokenIsFileSeparator = true;
                    }
                else {
                    nextLongerString =
                        autoSprintf( "%s %s ",
                                     currentLineString,
                                     *( tokens->getElement( 0 ) ) );
                    }
                
                }
                
            while( !nextTokenIsFileSeparator 
                   &&
                   nextLongerString != NULL 
                   && 
                   mainFont->measureString( nextLongerString ) 
                   < maxWidth 
                   &&
                   tokens->size() > 0 ) {
                    
                delete [] currentLineString;
                    
                currentLineString = nextLongerString;
                    
                nextLongerString = NULL;
                    
                // token consumed
                delete [] *( tokens->getElement( 0 ) );
                tokens->deleteElement( 0 );
                    
                if( tokens->size() > 0 ) {
                    
                    char *nextToken = *( tokens->getElement( 0 ) );
                
                    if( nextToken[0] == 28 ) {
                        nextTokenIsFileSeparator = true;
                        }
                    else {
                        nextLongerString =
                            autoSprintf( "%s%s ",
                                         currentLineString,
                                         *( tokens->getElement( 0 ) ) );
                        }
                    }
                }
                
            if( nextLongerString != NULL ) {    
                delete [] nextLongerString;
                }
                
            while( mainFont->measureString( currentLineString ) > 
                   maxWidth ) {
                    
                // single token that is too long by itself
                // simply trim it and discard part of it 
                // (user typing nonsense anyway)
                    
                currentLineString[ strlen( currentLineString ) - 1 ] =
                    '\0';
                }
                
            if( currentLineString[ strlen( currentLineString ) - 1 ] 
                == ' ' ) {
                // trim last bit of whitespace
                currentLineString[ strlen( currentLineString ) - 1 ] = 
                    '\0';
                }

                
            lines.push_back( currentLineString );

            
            if( nextTokenIsFileSeparator ) {
                // file separator

                // put a paragraph separator in
                lines.push_back( stringDuplicate( "---" ) );

                // token consumed
                delete [] *( tokens->getElement( 0 ) );
                tokens->deleteElement( 0 );
                }
            }   


        // all tokens deleted above
        delete tokens;


        double messageLineSpacing = 0.625 * (viewHeight / 15);
        
        int numLinesToSkip = lines.size() - maxLines;

        if( numLinesToSkip < 0 ) {
            numLinesToSkip = 0;
            }
        
        
        for( int i=0; i<numLinesToSkip-1; i++ ) {
            char *currentLineString = *( lines.getElement( i ) );
            delete [] currentLineString;
            }
        
        int lastSkipLine = numLinesToSkip - 1;

        if( lastSkipLine >= 0 ) {
            
            char *currentLineString = *( lines.getElement( lastSkipLine ) );

            // draw above and faded out somewhat

            doublePair lastSkipLinePos = messagePos;
            
            lastSkipLinePos.y += messageLineSpacing;

            setDrawColor( 1, 1, 0.5, 0.125 * pauseScreenFade );

            mainFont->drawString( currentLineString, 
                                   lastSkipLinePos, alignCenter );

            
            delete [] currentLineString;
            }
        

        setDrawColor( 1, 1, 0.5, pauseScreenFade );

        for( int i=numLinesToSkip; i<lines.size(); i++ ) {
            char *currentLineString = *( lines.getElement( i ) );
            
            if( false && lastSkipLine >= 0 ) {
            
                if( i == numLinesToSkip ) {
                    // next to last
                    setDrawColor( 1, 1, 0.5, 0.25 * pauseScreenFade );
                    }
                else if( i == numLinesToSkip + 1 ) {
                    // next after that
                    setDrawColor( 1, 1, 0.5, 0.5 * pauseScreenFade );
                    }
                else if( i == numLinesToSkip + 2 ) {
                    // rest are full fade
                    setDrawColor( 1, 1, 0.5, pauseScreenFade );
                    }
                }
            
            mainFont->drawString( currentLineString, 
                                   messagePos, alignCenter );

            delete [] currentLineString;
                
            messagePos.y -= messageLineSpacing;
            }
        }
        
        

    setDrawColor( 1, 1, 1, pauseScreenFade );

    messagePos = lastScreenViewCenter;

    messagePos.y -= 3.75 * ( viewHeight / 15 );
    //mainFont->drawString( translate( "pauseMessage3" ), 
    //                      messagePos, alignCenter );

    messagePos.y -= 0.625 * (viewHeight / 15);

    const char* quitMessageKey = "pauseMessage3";
    
    if( isQuittingBlocked() ) {
        quitMessageKey = "pauseMessage3b";
        }

    mainFont->drawString( translate( quitMessageKey ), 
                          messagePos, alignCenter );

    }



void deleteCharFromUserTypedMessage() {
    if( currentUserTypedMessage != NULL ) {
                    
        int length = strlen( currentUserTypedMessage );
        
        char fileSeparatorDeleted = false;
        if( length > 2 ) {
            if( currentUserTypedMessage[ length - 2 ] == 28 ) {
                // file separator with spaces around it
                // delete whole thing with one keypress
                currentUserTypedMessage[ length - 3 ] = '\0';
                fileSeparatorDeleted = true;
                }
            }
        if( !fileSeparatorDeleted && length > 0 ) {
            currentUserTypedMessage[ length - 1 ] = '\0';
            }
        }
    }




void drawFrame( char inUpdate ) {    


    if( !inUpdate ) {
        drawFrameNoUpdate( false );
            
        drawPauseScreen();
        
        if( !wasPaused ) {
            if( currentGamePage != NULL ) {
                currentGamePage->base_makeNotActive();
                }
            // fade out music during pause
            setMusicLoudness( 0 );
            }
        wasPaused = true;

        // handle delete key repeat
        if( holdDeleteKeySteps > -1 ) {
            holdDeleteKeySteps ++;
            
            if( holdDeleteKeySteps > stepsBetweenDeleteRepeat ) {        
                // delete repeat

                // platform layer doesn't receive event for key held down
                // tell it we are still active so that it doesn't
                // reduce the framerate during long, held deletes
                wakeUpPauseFrameRate();
                


                // subtract from messsage
                deleteCharFromUserTypedMessage();
                
                            

                // shorter delay for subsequent repeats
                stepsBetweenDeleteRepeat = (int)( 2/ frameRateFactor );
                holdDeleteKeySteps = 0;
                }
            }

        // fade in pause screen
        if( pauseScreenFade < 1 ) {
            pauseScreenFade += ( 1.0 / 30 ) * frameRateFactor;
        
            if( pauseScreenFade > 1 ) {
                pauseScreenFade = 1;
                }
            }
        

        return;
        }


    // not paused


    // fade pause screen out
    if( pauseScreenFade > 0 ) {
        pauseScreenFade -= ( 1.0 / 30 ) * frameRateFactor;
        
        if( pauseScreenFade < 0 ) {
            pauseScreenFade = 0;

            if( currentUserTypedMessage != NULL ) {

                // make sure it doesn't already end with a file separator
                // (never insert two in a row, even when player closes
                //  pause screen without typing anything)
                int lengthCurrent = strlen( currentUserTypedMessage );

                if( lengthCurrent < 2 ||
                    currentUserTypedMessage[ lengthCurrent - 2 ] != 28 ) {
                         
                        
                    // insert at file separator (ascii 28)
                    
                    char *oldMessage = currentUserTypedMessage;
                    
                    currentUserTypedMessage = autoSprintf( "%s %c ", 
                                                           oldMessage,
                                                           28 );
                    delete [] oldMessage;
                    }
                }
            }
        }    
    
    

    if( !firstDrawFrameCalled ) {
        
        // do final init step... stuff that shouldn't be done until
        // we have control of screen
        
        char *moveKeyMapping = 
            SettingsManager::getStringSetting( "upLeftDownRightKeys" );
    
        if( moveKeyMapping != NULL ) {
            char *temp = stringToLowerCase( moveKeyMapping );
            delete [] moveKeyMapping;
            moveKeyMapping = temp;
        
            if( strlen( moveKeyMapping ) == 4 &&
                strcmp( moveKeyMapping, "wasd" ) != 0 ) {
                // different assignment

                upKey = moveKeyMapping[0];
                leftKey = moveKeyMapping[1];
                downKey = moveKeyMapping[2];
                rightKey = moveKeyMapping[3];
                }
            delete [] moveKeyMapping;
            }


        firstDrawFrameCalled = true;
        }

    if( wasPaused ) {
        if( currentGamePage != NULL ) {
            currentGamePage->base_makeActive( false );
            }
        // fade music in
        if( ! musicOff ) {
            setMusicLoudness( 1.0 );
            }
        wasPaused = false;
        }
    
    
    if( getServerShutdown() ) {
        currentGamePage = finalMessagePage;
        finalMessagePage->setMessageKey( "serverShutdownMessage" );
        
        currentGamePage->base_makeActive( true );
        }
    else if( getPermadead() ) {
        currentGamePage = finalMessagePage;
        finalMessagePage->setMessageKey( "permadeadMessage" );

        currentGamePage->base_makeActive( true );
        }
    

    if( currentGamePage != NULL ) {
        
        currentGamePage->base_step();

        if( currentGamePage == loginPage ) {
            if( loginPage->getLoginDone() ) {
                
                currentGamePage = checkoutHousePage;
                currentGamePage->base_makeActive( true );
                }
            }
        else if( currentGamePage == checkoutHousePage || 
                 currentGamePage == paymentReportPage ) {
            // same code to handle either checkoutHousePage OR
            // paymentReportPage (which sometimes comes immediately between
            //  checkoutHousePage and editHousePage )

            char *wifeName = checkoutHousePage->getWifeName();

            if( checkoutHousePage->getReturnToMenu() ) {
                currentGamePage = menuPage;
                currentGamePage->base_makeActive( true );
                }
            else if( currentGamePage == checkoutHousePage &&
                     wifeName != NULL &&
                     checkoutHousePage->getPaymentCount() > 0 &&
                     ( checkoutHousePage->getYouPaidTotal() > 0 || 
                       checkoutHousePage->getWifePaidTotal() > 0 ) ) {
                
                paymentReportPage->setPayments( 
                    wifeName,
                    checkoutHousePage->getPaymentCount(),
                    checkoutHousePage->getYouPaidTotal(),
                    checkoutHousePage->getWifePaidTotal() );

                currentGamePage = paymentReportPage;
                currentGamePage->base_makeActive( true );
                }
            else if( currentGamePage == checkoutHousePage ||
                     paymentReportPage->getDone() ) {
                char *sonName = checkoutHousePage->getSonName();
                char *daughterName = checkoutHousePage->getDaughterName();

                char *houseMap = checkoutHousePage->getHouseMap();
                char *vaultContents = checkoutHousePage->getVaultContents();
                char *backpackContents = 
                    checkoutHousePage->getBackpackContents();
                char *galleryContents = 
                    checkoutHousePage->getGalleryContents();
                char *priceList = checkoutHousePage->getPriceList();
                int lootValue = checkoutHousePage->getLootValue();
                char mustSelfTest = checkoutHousePage->getMustSelfTest();
                int numberOfTapes = checkoutHousePage->getNumberOfTapes();
                
                if( wifeName != NULL && 
                    sonName != NULL && 
                    daughterName != NULL && 
                    houseMap != NULL && 
                    vaultContents != NULL &&
                    backpackContents != NULL && 
                    galleryContents != NULL && 
                    priceList != NULL ) {
                    
                    editHousePage->setWifeName( wifeName );
                    editHousePage->setSonName( sonName );
                    editHousePage->setDaughterName( daughterName );

                    // empty, because house just checked out an nothing
                    // bought/sold yet
                    editHousePage->setPurchaseList( "#" );
                    editHousePage->setSellList( "#" );
                    
                    editHousePage->setHouseMap( houseMap );
                    editHousePage->setVaultContents( vaultContents );
                    editHousePage->setBackpackContents( backpackContents );
                    editHousePage->setGalleryContents( galleryContents );
                    editHousePage->setPriceList( priceList );
                    editHousePage->setNumberOfTapes( numberOfTapes );
                    editHousePage->setLootValue( lootValue );
                    editHousePage->setMustSelfTest( mustSelfTest );
                    

                    delete [] sonName;
                    delete [] daughterName;
                    delete [] houseMap;
                    delete [] vaultContents;
                    delete [] backpackContents;
                    delete [] galleryContents;
                    delete [] priceList;

                    currentGamePage = editHousePage;
                    currentGamePage->base_makeActive( true );
                    }
                else {                    
                    if( sonName != NULL ) {
                        delete [] sonName;
                        }
                    if( daughterName != NULL ) {
                        delete [] daughterName;
                        }
                    if( houseMap != NULL ) {
                        delete [] houseMap;
                        }
                    if( vaultContents != NULL ) {
                        delete [] vaultContents;
                        }
                    if( backpackContents != NULL ) {
                        delete [] backpackContents;
                        }
                    if( galleryContents != NULL ) {
                        delete [] galleryContents;
                        }
                    if( priceList != NULL ) {
                        delete [] priceList;
                        }
                    }
                }

            if( wifeName != NULL ) {
                delete [] wifeName;
                }
            }
        else if( currentGamePage == editHousePage ) {
            if( editHousePage->isStale() ) {
                currentGamePage = staleHousePage;
                currentGamePage->base_makeActive( true );
                }
            else if( editHousePage->shouldJumpToTapes() ) {
                // abandon edit and jump right to tape list
                currentGamePage = robberyReplayMenuPage;
                // return to editing house after
                robberyReplayMenuPage->setEditHouseOnDone( true );
                currentGamePage->base_makeActive( true );
                }
            else if( editHousePage->showLoadBackpack() ) {
                char *vaultContents = editHousePage->getVaultContents();
                char *backpackContents = editHousePage->getBackpackContents();
                char *purchaseList = editHousePage->getPurchaseList();
                char *sellList = editHousePage->getSellList();
                char *priceList = editHousePage->getPriceList();
                
                loadBackpackPage->setLootValue( 
                    editHousePage->getLootValue() );
                loadBackpackPage->setVaultContents( vaultContents );
                loadBackpackPage->setBackpackContents( backpackContents );
                loadBackpackPage->setPurchaseList( purchaseList );
                loadBackpackPage->setSellList( sellList );
                loadBackpackPage->setPriceList( priceList );
                
                currentGamePage = loadBackpackPage;
                currentGamePage->base_makeActive( true );

                delete [] vaultContents;
                delete [] backpackContents;
                delete [] purchaseList;
                delete [] sellList;
                delete [] priceList;
                }
            else if( editHousePage->showGridObjectPicker() ) {
                objectPickerGridPage->setLootValue( 
                    editHousePage->getLootValue() );
                objectPickerGridPage->pullFromPicker( 
                    editHousePage->getObjectPicker() );
                
                currentGamePage = objectPickerGridPage;
                currentGamePage->base_makeActive( true );
                }
            else if( editHousePage->showAuctions() ) {
                // FIXME
                // char *galleryContents = editHousePage->getGalleryContents();
                
                // auctionPage->setGalleryContents( galleryContents );
                // delete [] galleryContents;

                auctionPage->setLootValue( editHousePage->getLootValue() );
                                
                currentGamePage = auctionPage;
                currentGamePage->base_makeActive( true );
                }
            else if( editHousePage->getDone() ) {
                
                char *wifeName = editHousePage->getWifeName();
                char *sonName = editHousePage->getSonName();
                char *daughterName = editHousePage->getDaughterName();
                
                char *houseMap = editHousePage->getHouseMap();
                char *vaultContents = editHousePage->getVaultContents();
                char *backpackContents = editHousePage->getBackpackContents();
                char *galleryContents = editHousePage->getGalleryContents();
                char *familyExitPaths = editHousePage->getFamilyExitPaths();
                char *purchaseList = editHousePage->getPurchaseList();
                char *sellList = editHousePage->getSellList();
                char *priceList = editHousePage->getPriceList();
                
                if( editHousePage->houseMapChanged() &&
                    ! editHousePage->getDead() ) {
                    // force player to test own house first
                    
                    selfHouseTestPage->setWifeName( wifeName );
                    selfHouseTestPage->setSonName( sonName );
                    selfHouseTestPage->setDaughterName( daughterName );
                    
                    selfHouseTestPage->setHouseMap( houseMap );
                    
                    selfHouseTestPage->setGalleryContents( galleryContents );
                    
                    selfHouseTestPage->setDescription( 
                        translate( "selfTestDescription" ) );

                    if( editHousePage->getWifeLiving() ) {    
                        // wife carries half money, rounded down
                        selfHouseTestPage->setWifeMoney(
                            editHousePage->getLootValue() / 2 );
                        }
                    else {
                        // dead wife carries nothing
                        selfHouseTestPage->setWifeMoney( 0 );
                        }

                    
                    // still present in checkoutHousePage, use it
                    // (editHousePage doesn't need it)
                    selfHouseTestPage->setMusicSeed( 
                        checkoutHousePage->getMusicSeed() );

                    blockQuitting( true );
                    currentGamePage = selfHouseTestPage;
                    currentGamePage->base_makeActive( true );
                    selfHouseTestPage->startSelfTest();
                    }
                else {
                    // not changed (or suicide, even if changed), 
                    // check it right in

                    // check for suicide (which can only happen on edit screen
                    // if house not edited, but can happen if self test forced,
                    // in which case we skip the self test)
                    if( editHousePage->getDead() ) {
                        checkinHousePage->setDied( 1 );
                        }
                    else {
                        // no self test needed
                        checkinHousePage->setDied( 2 );
                        }
                    
                    checkinHousePage->setHouseMap( houseMap );
                    checkinHousePage->setVaultContents( vaultContents );
                    checkinHousePage->setBackpackContents( backpackContents );
                    checkinHousePage->setGalleryContents( galleryContents );
                    checkinHousePage->setFamilyExitPaths( familyExitPaths );
                    checkinHousePage->setPurchaseList( purchaseList );
                    checkinHousePage->setSellList( sellList );
                    checkinHousePage->setPriceList( priceList );
                    checkinHousePage->setMoveList( "#" );
                    
                    currentGamePage = checkinHousePage;
                    currentGamePage->base_makeActive( true );
                    }

                delete [] wifeName;
                delete [] sonName;
                delete [] daughterName;

                delete [] houseMap;
                delete [] vaultContents;
                delete [] backpackContents;
                delete [] galleryContents;
                delete [] familyExitPaths;
                delete [] purchaseList;
                delete [] sellList;
                delete [] priceList;
                }
            }
        else if( currentGamePage == staleHousePage ) {
            if( staleHousePage->getDone() ) {
                // house edit became stale
                
                // return to menu
                currentGamePage = menuPage;
                currentGamePage->base_makeActive( true );
                }
            }
        else if( currentGamePage == staleHouseDeadPage ) {
            if( staleHouseDeadPage->getDone() ) {
                // either robbery or house self-test became stale
                
                // return to own, fresh house in either case
                currentGamePage = checkoutHousePage;
                currentGamePage->base_makeActive( true );
                }
            }
        else if( currentGamePage == loadBackpackPage ) {
            if( loadBackpackPage->isStale() ) {
                currentGamePage = staleHousePage;
                currentGamePage->base_makeActive( true );
                }
            else if( loadBackpackPage->showGridToolPicker() ) {
                toolPickerGridPage->setLootValue( 
                    loadBackpackPage->getLootValue() );
                toolPickerGridPage->pullFromPicker( 
                    loadBackpackPage->getToolPicker() );
                
                currentGamePage = toolPickerGridPage;
                currentGamePage->base_makeActive( true );
                }
            else if( loadBackpackPage->getDone() ) {
                // done changing backpack, back to editor
                
                char *vaultContents = loadBackpackPage->getVaultContents();
                char *backpackContents = 
                    loadBackpackPage->getBackpackContents();
                char *purchaseList = loadBackpackPage->getPurchaseList();
                char *sellList = loadBackpackPage->getSellList();
                
                char changeHappened = loadBackpackPage->getChangeHappened();
                if( changeHappened ) {
                    editHousePage->setBackpackOrVaultChanged();
                    }

                editHousePage->setVaultContents( vaultContents );
                editHousePage->setBackpackContents( backpackContents );
                editHousePage->setPurchaseList( purchaseList );
                editHousePage->setSellList( sellList );
                
                editHousePage->setLootValue( 
                    loadBackpackPage->getLootValue() );


                delete [] vaultContents;
                delete [] backpackContents;
                delete [] purchaseList;
                delete [] sellList;
                
                // back to editing
                currentGamePage = editHousePage;
                currentGamePage->base_makeActive( true );
                }
            }
        else if( currentGamePage == objectPickerGridPage ) {
            if( objectPickerGridPage->isStale() ) {
                currentGamePage = staleHousePage;
                currentGamePage->base_makeActive( true );
                }
            else if( objectPickerGridPage->getDone() ) {
                // object picked
                
                int newObject = objectPickerGridPage->getSelectedObject();
                
                if( newObject != -1 ) {    
                    editHousePage->getObjectPicker()->
                        setSelectedObject( newObject );
                    }
                
                // back to editing
                currentGamePage = editHousePage;
                currentGamePage->base_makeActive( true );
                }
            }
        else if( currentGamePage == toolPickerGridPage ) {
            if( toolPickerGridPage->isStale() ) {
                currentGamePage = staleHousePage;
                currentGamePage->base_makeActive( true );
                }
            else if( toolPickerGridPage->getDone() ) {
                // tool picked
                
                int newTool = toolPickerGridPage->getSelectedObject();
                
                if( newTool != -1 ) {    
                    loadBackpackPage->getToolPicker()->
                        setSelectedObject( newTool );
                    }
                
                // back to loading backpack
                currentGamePage = loadBackpackPage;
                currentGamePage->base_makeActive( true );
                }
            }
        else if( currentGamePage == auctionPage ) {
            if( auctionPage->isStale() ) {
                currentGamePage = staleHousePage;
                currentGamePage->base_makeActive( true );
                }
            else if( auctionPage->getDone() ) {
                // done with auctions, back to editor
                currentGamePage = editHousePage;
                currentGamePage->base_makeActive( true );
                }
            else if( auctionPage->getBuy() ) {
                buyAuctionPage->setObject( auctionPage->getBoughtObject() );
                buyAuctionPage->setLootValue( editHousePage->getLootValue() );

                char *galleryContents = editHousePage->getGalleryContents();
                buyAuctionPage->setGalleryContents( galleryContents );
                delete [] galleryContents;

                currentGamePage = buyAuctionPage;
                currentGamePage->base_makeActive( true );
                }
            }
        else if( currentGamePage == buyAuctionPage ) {
            if( buyAuctionPage->getReturnToHome() ) {                
                char *galleryContents = buyAuctionPage->getGalleryContents();
                editHousePage->setGalleryContents( galleryContents );
                delete [] galleryContents;
                                
                editHousePage->setLootValue( buyAuctionPage->getLootValue() );

                currentGamePage = editHousePage;
                currentGamePage->base_makeActive( true );
                }
            }
        else if( currentGamePage == selfHouseTestPage ) {
            if( selfHouseTestPage->didStartTestFail() ) {
                // starting self test failed (stale checkout)
                // but player was not in danger when this happened
                // (just started self test)
                // So don't display a dead message
                blockQuitting( false );
                currentGamePage = staleHousePage;
                currentGamePage->base_makeActive( true );
                clearNotes();
                }
            else if( selfHouseTestPage->isStale() ) {
                // house test stale, and starting the test didn't fail
                // player died from letting the self test go stale
                blockQuitting( false );
                currentGamePage = staleHouseDeadPage;
                staleHouseDeadPage->setOutOfTime( false );
                
                currentGamePage->base_makeActive( true );
                clearNotes();
                }
            else if( selfHouseTestPage->getDone() ) {
                char *houseMap = editHousePage->getHouseMap();
                char *vaultContents = editHousePage->getVaultContents();
                char *backpackContents = editHousePage->getBackpackContents();
                char *galleryContents = editHousePage->getGalleryContents();
                char *familyExitPaths = editHousePage->getFamilyExitPaths();
                char *purchaseList = editHousePage->getPurchaseList();
                char *sellList = editHousePage->getSellList();
                char *priceList = editHousePage->getPriceList();
                
                char *moveList = selfHouseTestPage->getMoveList();
                int testResult = selfHouseTestPage->getSuccess();

                if( testResult == 1 ||
                    testResult == 0 ) {                                        
                    // house passed by owner (reached vault), okay to check in
                    // OR died while testing, check in
                    checkinHousePage->setHouseMap( houseMap );
                    checkinHousePage->setVaultContents( vaultContents );
                    checkinHousePage->setBackpackContents( backpackContents );
                    checkinHousePage->setGalleryContents( galleryContents );
                    checkinHousePage->setFamilyExitPaths( familyExitPaths );
                    checkinHousePage->setPurchaseList( purchaseList );
                    checkinHousePage->setSellList( sellList );
                    checkinHousePage->setPriceList( priceList );
                    checkinHousePage->setMoveList( moveList );
                    
                    // test result = 0 if died, 1 if success
                    checkinHousePage->setDied( ! testResult );
                    
                    currentGamePage = checkinHousePage;
                    currentGamePage->base_makeActive( true );
                    }
                else {
                    // back to editing it
                    blockQuitting( false );
                    
                    // don't SET it again, because we want to keep
                    // track of the fact that it has changed since the
                    // last valid checkin
                    // editHousePage->setHouseMap( houseMap );
                    
                    editHousePage->endSelfTest();

                    currentGamePage = editHousePage;
                    currentGamePage->base_makeActive( true );
                    }
                delete [] houseMap;
                delete [] vaultContents;
                delete [] backpackContents;
                delete [] galleryContents;
                delete [] familyExitPaths;
                delete [] purchaseList;
                delete [] sellList;
                delete [] priceList;
                delete [] moveList;
                }
            }
        else if( currentGamePage == checkinHousePage ) {
            if( checkinHousePage->getReturnToMenu() ) {
                blockQuitting( false );
                currentGamePage = menuPage;
                currentGamePage->base_makeActive( true );
                }
            else if( checkinHousePage->getStartOver() ) {
                blockQuitting( false );
                // fresh start, back to editing empty house
                currentGamePage = checkoutHousePage;
                currentGamePage->base_makeActive( true );
                }
            }
        else if( currentGamePage == menuPage ) {
            if( menuPage->getStartEditHouse() ) {
                currentGamePage = checkoutHousePage;
                currentGamePage->base_makeActive( true );
                }
            if( menuPage->getShowReplayList() ) {
                currentGamePage = robberyReplayMenuPage;
                // return to menu after
                robberyReplayMenuPage->setEditHouseOnDone( false );
                currentGamePage->base_makeActive( true );
                }
            else if( menuPage->getStartSelfTestReplay() ) {
                HouseRecord *r = menuPage->getSelectedHouse();
                
                if( r != NULL ) {
                    currentGamePage = fetchSelfTestReplayPage;
                    
                    fetchSelfTestReplayPage->setOwnerID( r->uniqueID );
                    
                    currentGamePage->base_makeActive( true );
                    }
                }
            else if( menuPage->getStartRobHouse() ) {
                HouseRecord *r = menuPage->getSelectedHouse();
                
                if( r != NULL ) {
                    blockQuitting( true );
                    currentGamePage = robCheckoutHousePage;
                    
                    robCheckoutHousePage->setToRobHomeID( r->uniqueID );
                    robCheckoutHousePage->setToRobCharacterName( 
                        r->rawCharacterName );
                    
                    currentGamePage->base_makeActive( true );
                    }
                }
            else if( menuPage->getViewBlueprint() ) {
                HouseRecord *r = menuPage->getSelectedHouse();
                
                if( r != NULL ) {
                    currentGamePage = fetchBlueprintPage;
                    
                    fetchBlueprintPage->setToRobUserID( r->uniqueID );
                    fetchBlueprintPage->setToRobCharacterName( 
                        r->rawCharacterName );
                    
                    currentGamePage->base_makeActive( true );
                    }
                }
            }
        else if( currentGamePage == robCheckoutHousePage ) {
            if( robCheckoutHousePage->getReturnToMenu() ) {
                blockQuitting( false );
                currentGamePage = menuPage;
                currentGamePage->base_makeActive( true );
                }
            else {
                char *houseMap = robCheckoutHousePage->getHouseMap();
                
                if( houseMap != NULL ) {

                    robHousePage->setHouseMap( houseMap );
                    clearString( houseMap );

                    char *wifeName = robCheckoutHousePage->getWifeName();
                    robHousePage->setWifeName( wifeName );
                    delete [] wifeName;
                    
                    char *sonName = robCheckoutHousePage->getSonName();
                    robHousePage->setSonName( sonName );
                    delete [] sonName;
                    
                    char *daughterName = 
                        robCheckoutHousePage->getDaughterName();
                    robHousePage->setDaughterName( daughterName );
                    delete [] daughterName;
                    

                    char *backpackContents = 
                        robCheckoutHousePage->getBackpackContents();
                    
                    robHousePage->setBackpackContents( backpackContents );
                    
                    delete [] backpackContents;

                    char *galleryContents = 
                        robCheckoutHousePage->getGalleryContents();
                    
                    robHousePage->setGalleryContents( galleryContents );
                    
                    delete [] galleryContents;


                    char *ownerName = robCheckoutHousePage->getOwnerName();
                    
                    char *ownerNamePossessive = makePossessive( ownerName );

                    char *description = 
                        autoSprintf( translate( "robDescription" ),
                                     ownerNamePossessive );

                    delete [] ownerNamePossessive;


                    robHousePage->setDescription( description );
                    delete [] description;


                    robHousePage->setWifeMoney(
                        robCheckoutHousePage->getWifeMoney() );
                    
                    robHousePage->setMusicSeed( 
                        robCheckoutHousePage->getMusicSeed() );

                    robHousePage->setMaxSeconds( 
                        robCheckoutHousePage->getMaxSeconds() );

                    currentGamePage = robHousePage;
                    currentGamePage->base_makeActive( true );
                    }
                }
            }
        else if( currentGamePage == robHousePage ) {
            if( robHousePage->isStale() ) {
                blockQuitting( false );
                currentGamePage = staleHouseDeadPage;
                staleHouseDeadPage->setOutOfTime( 
                    robHousePage->isOutOfTime() );
                
                currentGamePage->base_makeActive( true );
                clearNotes();
                }
            else if( robHousePage->getViewBlueprint() ) {
                char *houseMap = robHousePage->getBlueprintMap();
                
                viewBlueprintDuringRobPage->setHouseMap( houseMap );
                delete [] houseMap;


                char *ownerName = robCheckoutHousePage->getOwnerName();
                    
                char *ownerNamePossessive = makePossessive( ownerName );
                    
                char *description = 
                    autoSprintf( translate( "blueprintDescription" ),
                                 ownerNamePossessive );

                delete [] ownerNamePossessive;


                viewBlueprintDuringRobPage->setDescription( description );
                delete [] description;

                viewBlueprintDuringRobPage->setViewOffset(
                    robHousePage->getVisibleOffsetX(),
                    robHousePage->getVisibleOffsetY() );


                currentGamePage = viewBlueprintDuringRobPage;
                currentGamePage->base_makeActive( true );
                }
            else if( robHousePage->getDone() ) {
                char *houseMap = robHousePage->getHouseMap();
                char *moveList = robHousePage->getMoveList();
                char *backpackContents = robHousePage->getBackpackContents();
                
                robCheckinHousePage->setBackpackContents( backpackContents );
                robCheckinHousePage->setHouseMap( houseMap );
                robCheckinHousePage->setSuccess( robHousePage->getSuccess() );

                robCheckinHousePage->setWifeKilledRobber( 
                    robHousePage->getWifeKilledRobber() );
                              
                robCheckinHousePage->setWifeKilled( 
                    robHousePage->getWifeKilled() );
                robCheckinHousePage->setWifeRobbed( 
                    robHousePage->getWifeRobbed() );

                robCheckinHousePage->setFamilyKilledCount( 
                    robHousePage->getFamilyKilledCount() );

                
                robCheckinHousePage->setMoveList( moveList );

                delete [] backpackContents;
                delete [] moveList;
                clearString( houseMap );
                
                currentGamePage = robCheckinHousePage;
                currentGamePage->base_makeActive( true );
                }
            }
        else if( currentGamePage == robCheckinHousePage ) {
            if( robCheckinHousePage->getReturnToHome() ) {
                blockQuitting( false );
                currentGamePage = checkoutHousePage;
                currentGamePage->base_makeActive( true );
                }
            else if( robCheckinHousePage->getStartOver() ) {
                blockQuitting( false );
                // fresh start, back to editing empty house
                currentGamePage = checkoutHousePage;
                currentGamePage->base_makeActive( true );
                }
            }
        else if( currentGamePage == robberyReplayMenuPage ) {
            if( robberyReplayMenuPage->getReturnToMenu() ) {
                currentGamePage = menuPage;
                currentGamePage->base_makeActive( true );
                }
            else if( robberyReplayMenuPage->getStartEditHouse() ) {
                currentGamePage = checkoutHousePage;
                currentGamePage->base_makeActive( true );
                }
            else if( robberyReplayMenuPage->getStartReplay() ) {
                int id = robberyReplayMenuPage->getLogID();
                
                if( id != -1  ) {
                    currentGamePage = fetchRobberyReplayPage;
                    fetchRobberyReplayPage->setLogID( id );
                    currentGamePage->base_makeActive( true );
                    }
                }
            }
        else if( currentGamePage == fetchRobberyReplayPage ) {
            if( fetchRobberyReplayPage->getReturnToMenu() ) {
                currentGamePage = menuPage;
                currentGamePage->base_makeActive( true );
                }
            else if( fetchRobberyReplayPage->getRecordReady() ) {
                RobberyLog log = fetchRobberyReplayPage->getLogRecord();
                
                replayRobHousePage->setLog( log );
                
                currentGamePage = replayRobHousePage;
                currentGamePage->base_makeActive( true );
                }
            }
        else if( currentGamePage == fetchSelfTestReplayPage ) {
            if( fetchSelfTestReplayPage->getReturnToMenu() ) {
                currentGamePage = menuPage;
                currentGamePage->base_makeActive( true );
                }
            else if( fetchSelfTestReplayPage->getRecordReady() ) {
                RobberyLog log = fetchSelfTestReplayPage->getLogRecord();
                
                replayRobHousePage->setLog( log );
                
                currentGamePage = replayRobHousePage;
                currentGamePage->base_makeActive( true );
                }
            }
        else if( currentGamePage == replayRobHousePage ) {
            if( replayRobHousePage->getDone() ) {
                // nothing to check in (just a read-only replay)

                // back to list of replays
                currentGamePage = robberyReplayMenuPage;
                currentGamePage->base_makeActive( true );
                }
            }
        else if( currentGamePage == fetchBlueprintPage ) {
            if( fetchBlueprintPage->getReturnToMenu() ) {
                currentGamePage = menuPage;
                currentGamePage->base_makeActive( true );
                }
            else {
                char *houseMap = fetchBlueprintPage->getHouseMap();
                
                if( houseMap != NULL ) {

                    viewBlueprintPage->setHouseMap( houseMap );
                    delete [] houseMap;


                    char *ownerName = fetchBlueprintPage->getOwnerName();
                    
                    char *ownerNamePossessive = makePossessive( ownerName );

                    char *description = 
                        autoSprintf( translate( "blueprintDescription" ),
                                     ownerNamePossessive );

                    delete [] ownerNamePossessive;


                    viewBlueprintPage->setDescription( description );
                    delete [] description;

                    currentGamePage = viewBlueprintPage;
                    currentGamePage->base_makeActive( true );
                    }
                }
            }
        else if( currentGamePage == viewBlueprintPage ) {
            if( viewBlueprintPage->getDone() ) {

                currentGamePage = menuPage;
                currentGamePage->base_makeActive( true );
                }
            }
        else if( currentGamePage == viewBlueprintDuringRobPage ) {
            if( viewBlueprintDuringRobPage->getDone() ) {

                currentGamePage = robHousePage;

                // keep music going, etc.
                // not a fresh jump to active for rob page
                currentGamePage->base_makeActive( false );
                }
            }


        }


    // now draw stuff AFTER all updates
    drawFrameNoUpdate( true );



    // draw tail end of pause screen, if it is still visible
    if( pauseScreenFade > 0 ) {
        drawPauseScreen();
        }
    }



void drawFrameNoUpdate( char inUpdate ) {

    if( currentGamePage != NULL ) {
        currentGamePage->base_draw( lastScreenViewCenter, viewWidth );
        }

    }



// store mouse data for use as unguessable randomizing data
// for key generation, etc.
#define MOUSE_DATA_BUFFER_SIZE 20
int mouseDataBufferSize = MOUSE_DATA_BUFFER_SIZE;
int nextMouseDataIndex = 0;
// ensure that stationary mouse data (same value over and over)
// doesn't overwrite data from actual motion
float lastBufferedMouseValue = 0;
float mouseDataBuffer[ MOUSE_DATA_BUFFER_SIZE ];



void pointerMove( float inX, float inY ) {

    // save all mouse movement data for key generation
    float bufferValue = inX + inY;
    // ignore mouse positions that are the same as the last one
    // only save data when mouse actually moving
    if( bufferValue != lastBufferedMouseValue ) {
        
        mouseDataBuffer[ nextMouseDataIndex ] = bufferValue;
        lastBufferedMouseValue = bufferValue;
        
        nextMouseDataIndex ++;
        if( nextMouseDataIndex >= mouseDataBufferSize ) {
            nextMouseDataIndex = 0;
            }
        }
    

    if( isPaused() ) {
        return;
        }
    
    if( currentGamePage != NULL ) {
        currentGamePage->base_pointerMove( inX, inY );
        }
    }



void pointerDown( float inX, float inY ) {
    if( isPaused() ) {
        return;
        }
    
    if( currentGamePage != NULL ) {
        currentGamePage->base_pointerDown( inX, inY );
        }
    }



void pointerDrag( float inX, float inY ) {
    if( isPaused() ) {
        return;
        }
    
    if( currentGamePage != NULL ) {
        currentGamePage->base_pointerDrag( inX, inY );
        }
    }



void pointerUp( float inX, float inY ) {
    if( isPaused() ) {
        return;
        }
    
    if( currentGamePage != NULL ) {
        currentGamePage->base_pointerUp( inX, inY );
        }
    }







void keyDown( unsigned char inASCII ) {

    // taking screen shot is ALWAYS possible
    if( inASCII == '=' ) {    
        saveScreenShot( "screen" );
        }
    

    
    if( isPaused() ) {
        // block general keyboard control during pause


        switch( inASCII ) {
            case 13:  // enter
                // unpause
                pauseGame();
                break;
            }
        
        
        if( inASCII == 127 || inASCII == 8 ) {
            // subtract from it

            deleteCharFromUserTypedMessage();

            holdDeleteKeySteps = 0;
            // start with long delay until first repeat
            stepsBetweenDeleteRepeat = (int)( 30 / frameRateFactor );
            }
        else if( inASCII >= 32 ) {
            // add to it
            if( currentUserTypedMessage != NULL ) {
                
                char *oldMessage = currentUserTypedMessage;

                currentUserTypedMessage = autoSprintf( "%s%c", 
                                                       oldMessage, inASCII );
                delete [] oldMessage;
                }
            else {
                currentUserTypedMessage = autoSprintf( "%c", inASCII );
                }
            }
        
        return;
        }
    
    
    if( currentGamePage != NULL ) {
        currentGamePage->base_keyDown( inASCII );
        }

    
    switch( inASCII ) {
        case 'm':
        case 'M': {
#ifdef USE_MALLINFO
            struct mallinfo meminfo = mallinfo();
            printf( "Mem alloc: %d\n",
                    meminfo.uordblks / 1024 );
#endif
            }
            break;
        }
    }



void keyUp( unsigned char inASCII ) {
    if( inASCII == 127 || inASCII == 8 ) {
        // delete no longer held
        // even if pause screen no longer up, pay attention to this
        holdDeleteKeySteps = -1;
        }

    if( ! isPaused() ) {
        if( currentGamePage != NULL ) {
            currentGamePage->base_keyUp( inASCII );
            }
        }

    }







void specialKeyDown( int inKey ) {
    if( isPaused() ) {
        return;
        }
    

    if( currentGamePage != NULL ) {
        currentGamePage->base_specialKeyDown( inKey );
        }

	}



void specialKeyUp( int inKey ) {
    if( isPaused() ) {
        return;
        }
    

    if( currentGamePage != NULL ) {
        currentGamePage->base_specialKeyUp( inKey );
        }
	} 




char getUsesSound() {
    return true;
    }









void drawString( const char *inString, char inForceCenter ) {
    
    setDrawColor( 1, 1, 1, 0.75 );

    doublePair messagePos = lastScreenViewCenter;

    TextAlignment align = alignCenter;
    
    if( initDone && !inForceCenter ) {
        // transparent message
        setDrawColor( 1, 1, 1, 0.75 );

        // stick messages in corner
        messagePos.x -= viewWidth / 2;
        
        messagePos.x +=  0.25;
    

    
        messagePos.y += (viewWidth * viewHeightFraction) /  2;
    
        messagePos.y -= 0.4375;
        messagePos.y -= 0.5;

        align = alignLeft;
        }
    else {
        // fully opaque message
        setDrawColor( 1, 1, 1, 1 );

        // leave centered
        }
    

    int numLines;
    
    char **lines = split( inString, "\n", &numLines );
    
    for( int i=0; i<numLines; i++ ) {
        

        mainFont->drawString( lines[i], messagePos, align );
        messagePos.y -= 0.75;
        
        delete [] lines[i];
        }
    delete [] lines;
    }




