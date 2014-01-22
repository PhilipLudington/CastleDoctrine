#include "minorGems/game/game.h"

#include "minorGems/sound/filters/SoundFilter.h"


// dimensions of note grid
// tone height
// two pentatonic octaves
#define N  10
// time width
#define NW 32

// instrument parts


#define PARTS 4

// last part (PARTS-1) doesn't have its own timbre/envelope, but instead
// uses pointers to one of other part's timber/envelope
extern char noteToggles[PARTS][N][NW];
extern int partLengths[PARTS];

extern double partLoudness[PARTS];
extern double partStereo[PARTS];




void initMusicPlayer();
void freeMusicPlayer();






// set loudness in range [0.0,1.0]
// starts at 0 (so must be increased after initMusicPlayer to hear music)
// music loudness is adjusted toward inLoudness smoothly 
// Adjustment rate covers a full 0-to-1 volume change in 1 second
void setMusicLoudness( double inLoudness );


double getMusicLoudness();


// returns all timbres/evelopes to those set by default
void setDefaultMusicSounds();


// inNumCoefficients must not exceed 256
void setTimbre( int inTimbreNumber, 
                double *inPartialCoefficients, int numCoefficients,
                int inOctavesDown );




// release + attack + hold must be <= 1.0
// sustains at full volume between attack and release slopes (no decay)
void setEnvelope( int inTimbreNumber,
                  double inAttack, double inHold,
                  double inRelease );



// sets scale 
// NOTE:  all timbres must be regenerated for this to have an effect
void setScale( char inToneOn[12] );


// 0, 1, 2, slow, med, fast    
// NOTE:  all envelopes must be regenerated for this to 
// have an effect
void setSpeed( int inSpeed );



// causes music to jump back to beginning
void restartMusic();



void stopMusic();






// Specify a filter that samples are passed through before being returned
// to audio device.
// Subsequent calls add filters that are applied after filters specified
// by earlier calls.
//
// Filters are destroyed when music player is freed.
void addMusicFilter( SoundFilter *inFilter );
