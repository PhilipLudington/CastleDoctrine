#include "gameElements.h"



static char readDescription( char *inContents, char **outDescription ) {
    char completeRecord = true;
    
    // skip the first "
    int readChar = ' ';

    while( readChar != '"' && readChar != '\0' ) {
        readChar = inContents[0];
        inContents = &( inContents[1] );
        }

                
    char *descriptionString = new char[1000];
    // scan a string of up to 999 characters, stopping
    // at the first " character
    int numRead = sscanf( inContents, "%999[^\"]",
                          descriptionString );
                

    if( numRead == 1 ) {
        *outDescription = stringDuplicate( descriptionString );
        }
    else {
        *outDescription = NULL;
        completeRecord = false;
        }
    delete [] descriptionString;
    
    return completeRecord;
    }




char readInfoFile( File *inFile, int *outID, char **outDescription ) {
    char completeRecord = true;
    
    if( inFile->exists() ) {
                
        char *info = inFile->readFileContents();

        int numRead = sscanf( info, "%d", outID );
                
        if( numRead != 1 ) {
            delete [] info;
            return false;
            }
        
        completeRecord = readDescription( info, outDescription );
        
        delete [] info;
        }
    else {
        completeRecord = false;
        }


    return completeRecord;
    }




char readPluralFile( File *inFile, char **outDescriptionPlural ) {
    char completeRecord = true;
    
    if( inFile->exists() ) {
                
        char *info = inFile->readFileContents();
                
        completeRecord = readDescription( info, outDescriptionPlural );
        
        delete [] info;
        }
    else {
        completeRecord = false;
        }


    return completeRecord;
    }

