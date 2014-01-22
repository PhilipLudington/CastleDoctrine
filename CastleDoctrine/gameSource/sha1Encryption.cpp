#include "sha1Encryption.h"
#include "sharedServerSecret.h"

#include "minorGems/formats/encodingUtils.h"
#include "minorGems/crypto/hashes/sha1.h"

#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/stringUtils.h"



char *sha1Decrypt( const char *inKey, const char *inEncryptedDataBase64 ) {
    int encryptedLength;
    
    unsigned char *encryptedData = base64Decode( (char*)inEncryptedDataBase64,
                                                 &encryptedLength );    
    
    SimpleVector<unsigned char> keyStream;
    
    int counter = 0;
    
    while( keyStream.size() < encryptedLength ) {
        
        char *stringToHash = autoSprintf( "%d%s%s%d", 
                                          counter, 
                                          inKey, sharedServerSecret,
                                          counter );
        
        unsigned char *hashData = computeRawSHA1Digest( stringToHash );
        
        delete [] stringToHash;
        
        keyStream.push_back( hashData, 20 );

        delete [] hashData;

        counter++;
        }
    

    char *decryptedData = new char[ encryptedLength + 1 ];
    
    unsigned char *keyStreamData = keyStream.getElementArray();
    

    for( int i=0; i<encryptedLength; i++ ) {    
        decryptedData[i] = keyStreamData[i] ^ encryptedData[i];
        }
    decryptedData[encryptedLength] = '\0';
    
    
    delete [] encryptedData;
    delete [] keyStreamData;
    

    return decryptedData;
    }



char *sha1Encrypt( const char *inKey, const char *inDataToEncrypt ) {
    int dataLength = strlen( inDataToEncrypt );

    SimpleVector<unsigned char> keyStream;
    
    int counter = 0;
    
    while( keyStream.size() < dataLength ) {
        
        char *stringToHash = autoSprintf( "%d%s%s%d", 
                                          counter, 
                                          inKey, sharedServerSecret,
                                          counter );
        
        unsigned char *hashData = computeRawSHA1Digest( stringToHash );
        
        delete [] stringToHash;
        
        keyStream.push_back( hashData, 20 );

        delete [] hashData;

        counter++;
        }
    

    char *encryptedData = new char[ dataLength + 1 ];
    
    unsigned char *keyStreamData = keyStream.getElementArray();
    

    for( int i=0; i<dataLength; i++ ) {    
        encryptedData[i] = keyStreamData[i] ^ inDataToEncrypt[i];
        }
    encryptedData[dataLength] = '\0';
    
    
    delete [] keyStreamData;
    

    char *returnString = base64Encode( (unsigned char *)encryptedData, 
                                       dataLength, false );
    
    delete [] encryptedData;


    return returnString;
    }
