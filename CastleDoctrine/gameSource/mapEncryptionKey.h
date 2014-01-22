


// generates an encryption key based on global mouse movement data
// this ensures that every map is encrypted with a different key, thwarting
// static tools that might mine the recordedGame files for house maps
// (because the encryption keys themselves don't end up in the recordedGame 
//  files).
char *getMapEncryptionKey();
