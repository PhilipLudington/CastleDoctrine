#include <string.h>

void clearString( char *inString ) {
    memset( inString, 0, strlen( inString ) );
    delete [] inString;
    }

