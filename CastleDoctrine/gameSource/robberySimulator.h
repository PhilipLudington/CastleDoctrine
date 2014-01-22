

void initSimulator();

void freeSimulator();


// takes in request as described in server/headlessProtocol.txt
// returns response also described in that file.
// (Without the [END_REQUEST] or [END_RESPONSE] markers.) 
// response is FAILED if robbery contains an illegal move
char *simulateRobbery( const char *inRobberyData );



