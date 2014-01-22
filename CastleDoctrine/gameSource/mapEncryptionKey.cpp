#include "mapEncryptionKey.h"

#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SimpleVector.h"

#include "minorGems/crypto/hashes/sha1.h"


// use these for unguessable data (key generation)
extern int mouseDataBufferSize;
extern float mouseDataBuffer[];


char *getMapEncryptionKey() {
    
    SimpleVector<char> keyGenCharacters;
    
    for( int b=0; b<mouseDataBufferSize; b++ ) {
        char *dataString = autoSprintf( "%f", mouseDataBuffer[b] );
        keyGenCharacters.push_back( dataString, strlen( dataString ) );
        delete [] dataString;
        }
    
    char *keyGenString = keyGenCharacters.getElementString();

    char *returnValue = computeSHA1Digest( keyGenString );
    
    delete [] keyGenString;

    return returnValue;
    }

