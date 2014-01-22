


// NOTE:  SHA1 was not meant to be used as a block cipher like this.
// I get it.
// But it is super light weight, and available everywhere.
// In this application, which is just meant to keep raw map data out
// of the recordedGame files and out of the protocol, it's sufficient.


// decrypts data that was encrypted with key-and-counter based SHA1 keystream
// result destroyed by caller.
// makes use of sharedServerSecret.
char *sha1Decrypt( const char *inKey, const char *inEncryptedDataBase64 );


// returns base64 string with encrypted data
// result destroyed by caller.
// makes use of sharedServerSecret.
char *sha1Encrypt( const char *inKey, const char *inDataToEncrypt );




