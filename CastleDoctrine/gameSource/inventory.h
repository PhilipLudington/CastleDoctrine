#ifndef INVENTORY_INCLUDED
#define INVENTORY_INCLUDED

// shared utility function for decoding/encoding inventories


#include "InventorySlotButton.h"

#include "minorGems/util/SimpleVector.h"



#define NUM_PACK_SLOTS 8

// must be a whole multiple of NUM_PACK_SLOTS
#define NUM_VAULT_SLOTS 48




typedef struct QuantityRecord {
        int objectID;
        int quantity;
    } QuantityRecord;




// parses a string in the following form:
// id:quanity#id:quanity#id:quantity (or # for an empty list)
//
// and adds the pairs as recors in a empty record list
void fromString( char *inListString, 
                 SimpleVector<QuantityRecord> *inEmptyList );

// converts a list of QuantityRecords to a string like
//  id:quanity#id:quanity#id:quantity (or # for an empty list)
char *toString( SimpleVector<QuantityRecord> *inQuantities );



// adds 1 (default) to an exisiting quantity record for inAddObjectID, 
// if it exists, 
// or appends a new record for inAddObjectID
void addToQuantity( SimpleVector<QuantityRecord> *inOldQuanties, 
                    int inAddObjectID,
                    int inAddition = 1 );

// subtracts 1 (default) from an existing quantity record for 
// inSubtractObjectID, if found
// returns true if found
char subtractFromQuantity( SimpleVector<QuantityRecord> *inOldQuanties,
                           int inSubtractObjectID,
                           int inSubtraction = 1 );



// parses a string representation inventory contents, and puts
// result into inventory slots (replacing their contents)
void inventorySlotsFromString( 
    char *inString,
    InventorySlotButton *inSlots[], int inNumSlots );


// encodes slots as a string (destroyed by caller)
// (works for both backpack and vault)
char *stringFromInventorySlots( 
    InventorySlotButton *inSlots[], int inNumSlots );



#endif
