#include "inventory.h"

#include "minorGems/util/stringUtils.h"




void fromString( char *inListString, 
                 SimpleVector<QuantityRecord> *inEmptyList ) {
    
    // empty pack represented by "#"
    if( strcmp( inListString, "#" ) != 0 ) {
        int numParts;
        char **parts = split( inListString, "#", &numParts );

        for( int i=0; i<numParts; i++ ) {
            int id, quantity;
            int numRead = sscanf( parts[i], "%d:%d", &id, &quantity  );
                
            if( numRead == 2 ) {
                QuantityRecord r = { id, quantity };
                inEmptyList->push_back( r );
                }
            delete [] parts[i];
            }
        delete [] parts;
        }
    }



char *toString( SimpleVector<QuantityRecord> *inQuantities ) {
    
    
    SimpleVector<char*> stringParts;
    for( int i=0; i<inQuantities->size(); i++ ) {
        
        QuantityRecord *r = inQuantities->getElement( i );
        
        char *part = autoSprintf( "%d:%d", r->objectID, r->quantity );
            
        stringParts.push_back( part );
        }


    if( stringParts.size() > 0 ) {
        
        char **partArray = stringParts.getElementArray();
    
        char *contentsString = join( partArray, stringParts.size(), "#" );
        
        delete [] partArray;
        
        for( int i=0; i<stringParts.size(); i++ ) {
            delete [] *( stringParts.getElement( i ) );
            }
        
        return contentsString;
        }
    else {
        // empty pack represented by "#"
        return stringDuplicate( "#" );
        }

    }



void addToQuantity( SimpleVector<QuantityRecord> *inOldQuanties, 
                    int inAddObjectID, int inAddition ) {
    
    char found = false;
    for( int j=0; j<inOldQuanties->size(); j++ ) {
        QuantityRecord *r = inOldQuanties->getElement( j );
        
        if( r->objectID == inAddObjectID ) {
            r->quantity += inAddition;
            found = true;
            break;
            }
        }
    
    if( !found ) {
        // new record for this object ID
        
        QuantityRecord r = { inAddObjectID, inAddition };
        inOldQuanties->push_back( r );
        }
    }



char subtractFromQuantity( SimpleVector<QuantityRecord> *inOldQuanties, 
                           int inSubtractObjectID, int inSubtraction ) {
    
    for( int j=0; j<inOldQuanties->size(); j++ ) {
        QuantityRecord *r = inOldQuanties->getElement( j );
        
        if( r->objectID == inSubtractObjectID ) {
            r->quantity -= inSubtraction;

            if( r->quantity == 0 ) {
                inOldQuanties->deleteElement( j );
                }
            return true;
            }
        }
    return false;
    }





void inventorySlotsFromString( 
    char *inString,
    InventorySlotButton *inSlots[], int inNumSlots ) {

    for( int i=0; i<inNumSlots; i++ ) {
        // clear slots
        inSlots[i]->setObject( -1 );
        }

    SimpleVector<QuantityRecord> tempRecords;
    
    fromString( inString, &tempRecords );
    
    for( int i=0; i<tempRecords.size() && i < inNumSlots; i++ ) {
        QuantityRecord *r = tempRecords.getElement( i );
        
        inSlots[i]->setObject( r->objectID );
        inSlots[i]->setQuantity( r->quantity );
        }
    }





char *stringFromInventorySlots( 
    InventorySlotButton *inSlots[], int inNumSlots ) {
    

    SimpleVector<QuantityRecord> quantities;
    
    for( int i=0; i<inNumSlots; i++ ) {
        int id = inSlots[i]->getObject();
        
        if( id != -1 ) {
            addToQuantity( &quantities, id, inSlots[i]->getQuantity() );
            }
        }

    return toString( &quantities );
    }
