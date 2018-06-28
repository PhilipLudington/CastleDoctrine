#include "ticketHash.h"
#include "sharedServerSecret.h"


#include "minorGems/util/stringUtils.h"
#include "minorGems/crypto/hashes/sha1.h"


extern char *downloadCode;
extern int serverSequenceNumber;
extern int ticketHashVersionNumber;


char *getTicketHash() {

    const char *codeToHash = "";

    if( downloadCode != NULL ) {
        codeToHash = downloadCode;
        }

    
    // strip out "-" separators
    int numParts;
    char **codeParts = split( codeToHash, "-", &numParts );
    
    char *pureCode =
        join( codeParts, numParts, "" );
    
    for( int i=0; i<numParts; i++ ) {
        delete [] codeParts[i];
        }
    delete [] codeParts;



    char *toHash = autoSprintf( "%d%d%s", 
                                serverSequenceNumber,
                                ticketHashVersionNumber,
                                sharedServerSecret );
    
    char *hash = hmac_sha1( pureCode, toHash );
    
    delete [] pureCode;
    delete [] toHash;

    char *result = autoSprintf( "sequence_number=%d&ticket_hmac=%s",
                                serverSequenceNumber,
                                hash );
    delete [] hash;

    serverSequenceNumber++;
    
    return result;
    }



char *replaceTicketHash( char *inString ) {
    const char *keyA = "sequence_number=";
    const char *keyB = "ticket_hmac=";
    
    if( inString == NULL ) {
        return NULL;
        }
    else if( strstr( inString, keyA ) != NULL &&
             strstr( inString, keyB ) != NULL ) {
        
        // present

        char *copy = stringDuplicate( inString );
        
        char *startPointerA = strstr( copy, keyA );
        char *startPointerB = strstr( copy, keyB );
 
        char *hmacStart = &( startPointerB[ strlen( keyB ) ] );
        
        
        int i = 0;
        while( hmacStart[i] != '\0' &&
               ( ( hmacStart[i] >= '0' && 
                   hmacStart[i] <= '9' )
                 || 
                 ( hmacStart[i] >= 'A' &&
                   hmacStart[i] <= 'F' ) ) ) {
            i++;
            }
        
        // truncate here, after end of hmac
        hmacStart[i] = '\0';
        
        
        // now startPointerA points to the full hash
        
        char *newHash = getTicketHash();

        char found;
        
        char *newHashInPlace = replaceOnce( inString, startPointerA,
                                            newHash,
                                            &found );

        delete [] newHash;
        delete [] copy;
        delete [] inString;

        return newHashInPlace;
        }
    else {
        // no hash present
        char *result = stringDuplicate( inString );
        delete [] inString;
        return result;
        }
    }

        
