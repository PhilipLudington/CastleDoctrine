#ifndef BLUEPRINT_HOUSE_GRID_DISPLAY_INCLUDED
#define BLUEPRINT_HOUSE_GRID_DISPLAY_INCLUDED


#include "HouseGridDisplay.h"

#include "minorGems/util/SimpleVector.h"


class BlueprintHouseGridDisplay : public HouseGridDisplay {
    
    public:

        BlueprintHouseGridDisplay( double inX, double inY );

        
        // override to switch to blueprint state
        virtual void setHouseMap( const char *inHouseMap );


        // override movement to pan around the map
        virtual void specialKeyDown( int inKeyCode );
        
    };


#endif
