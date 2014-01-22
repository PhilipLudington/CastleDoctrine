


// gets URL parameters for correct ticket hash based on download code
// and current sequence number.
//
// Example result:  
// sequence_number=36&ticket_hmac=8C87B150ACFBFE18B9790285986F169F67D7E82D
char *getTicketHash();


// replaces a ticket hash in inString with a fresh one
// deletes inString, if not NULL, and returns newly allocated string
// returns NULL if inString NULL
char *replaceTicketHash( char *inString );

