
#include "minorGems/util/random/CustomRandomSource.h"

#include "musicPlayer.h"


static CustomRandomSource musicRandSource;



void clearNotes() {
    lockAudio();
    for( int p=0; p<PARTS; p++ ) {
        for( int y=0; y<N; y++ ) {
            for( int x=0; x<NW; x++ ) {
                noteToggles[p][y][x] = false;
                }
            }
        }
    unlockAudio();
    }


void setMusicFromSeed( int inSeed ) {
    
    // first:
    // all notes off
    clearNotes();


    if( inSeed == 0 ) {
        return;
        }
    
    musicRandSource.reseed( inSeed );

    lockAudio();

    
    for( int p=0; p<PARTS; p++ ) {
    
        // parts randomly shorter or longer 
        // force unique part lengths (phase changes)
        char hit = true;
        while( hit ) {
            partLengths[p] = 
                NW + musicRandSource.getRandomBoundedInt( -NW/4, NW/4 );
            hit = false;
            for( int q=0; q<p; q++ ) {
                if( partLengths[p] == partLengths[q] ) {
                    // part length already taken
                    // pick again
                    hit = true;
                    break;
                    }
                }
            }
        }

    for( int p=0; p<PARTS; p++ ) {    
        
        for( int i=0; i<3; i++ ) {
            // pick column at random
            int range = NW;
            if( partLengths[p] < range ) {
                range = partLengths[p];
                }

            // avoid intra-part chords
            // keep picking x until we find an empty column
            char conflict = true;
            int x;
            while( conflict ) {
                
                x = musicRandSource.getRandomBoundedInt( 0, range - 1 );
        
                conflict = false;
                
                for( int y=0; y<N; y++ ) {
                    if( noteToggles[p][y][x] ) {
                        conflict = true;
                        break;
                        }
                    }
                }
            
            // pick pitch
            // never repeat a pitch
            conflict = true;
            int y;
            while( conflict ) {
                y = musicRandSource.getRandomBoundedInt( 0, N - 1 );
                conflict = false;
                
                for( int x=0; x<NW; x++ ) {
                    if( noteToggles[p][y][x] ) {
                        conflict = true;
                        break;
                        }
                    }
                }
            
            noteToggles[p][y][x] = true;
            }
        }

    



    unlockAudio();
    
    restartMusic();
    }

