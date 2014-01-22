
// replaces "_" with " " and handles display of You name
//
// inNameString destroyed internally
// result destroyed by caller
char *nameParse( char *inNameString );



// changes into posessive form
// handles names that end with "s" properly
//
// inNameString destroyed internally
// result destroyed by caller
char *makePossessive( char *inNameString );
