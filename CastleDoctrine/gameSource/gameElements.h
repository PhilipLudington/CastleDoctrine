
#include "minorGems/io/file/File.h"




// Parses info.txt file for either a house object or a tool
//
// outID is pointer to where ID should be returned
// outDescription is pointer to where newly allocated string (destroyed by 
//                caller) should be returned.
// Returns true on success
char readInfoFile( File *inFile, int *outID, char **outDescription );



// Parses plural.txt file
//
// outDescriptionPlural is pointer to where newly allocated string 
//                (destroyed by caller) should be returned.
// Returns true on success
char readPluralFile( File *inFile, char **outDescriptionPlural );
