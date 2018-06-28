#ifndef HOUSE_OBJECT_PICKER_INCLUDED
#define HOUSE_OBJECT_PICKER_INCLUDED



#include "PageComponent.h"

#include "SpriteButton.h"



typedef struct ObjectPriceRecord {
        int id;
        int price;
        } ObjectPriceRecord;


// fires actionPerformed when selection changes
class HouseObjectPicker : public PageComponent, public ActionListener, 
                          public ActionListenerList {
        
    public:
        
        // if inTools is true, only tools shown
        // else only house objects (non-tools) shown
        HouseObjectPicker( double inX, double inY,
                           char inTools = false );
        

        virtual ~HouseObjectPicker();
        
        
        virtual void actionPerformed( GUIComponent *inTarget );
        
        virtual char shouldShowGridView();
        
        
        virtual int getSelectedObject();
        
        // moves selected object to the top of the stack
        virtual void useSelectedObject();

        virtual void setSelectedObject( int inObjectID );


        // the prices in this list determine what is shown on picker
        // Thus, price list from server can override what objects are
        // shown in picker (by pairing down list only, not adding to it).
        virtual void setPriceList( const char *inPriceList );
        
        
        // inRecords destroyed by caller
        virtual void setPrices( ObjectPriceRecord *inRecords, 
                                int inNumRecords );

        // copied internally
        virtual void setWifeName( const char *inWifeName );

        virtual const char *getWifeName();
        
        // gets the price list in the original, server-provided order
        virtual ObjectPriceRecord *getPrices( int *outNumPrices );
        


        // returns -1 if no price set
        virtual int getPrice( int inObjectID );

        // returns -1 if no sell-back price set
        virtual int getSellBackPrice( int inObjectID );


        virtual void step();
        
        virtual void draw();
        
        virtual void setVisible( char inIsVible );

        
        char isDropDownOpen();
        int getDropDownNumCells();
        

    protected:
        virtual void clearState() {
            mHover = false;
            mDragOver = false;
            }
        
        virtual void pointerMove( float inX, float inY );
        virtual void pointerDrag( float inX, float inY );
        virtual void pointerUp( float inX, float inY );
        virtual void pointerDown( float inX, float inY );
        
        char mHover;
        char mDragOver;
        
        char mDropDownOpen;
        char mHoverIndex;

        char mShowTools;
        double mSpriteScale;
        char mShouldShowGridView;
        
        SimpleVector<ObjectPriceRecord> mObjectList;
        // original order
        SimpleVector<ObjectPriceRecord> mOriginalObjectList;
        // to block server-listed objects that we don't have locally
        SimpleVector<int> mLocalPresentIDs;
        
        int mSelectedIndex;
        
        double mPixWidth;

        SpriteButton mUpButton;
        SpriteButton mDownButton;
        SpriteButton mGridViewButton;
        
        SpriteHandle mMoreSprite;


        char *mWifeName;

        void triggerToolTip();
        
        char isInside( float inX, float inY );
        
        
        void drawBox( doublePair inPosition, double inVerticalRadius = 1 );
        void drawObjectInBox( int inSelectedIndex, doublePair inPosition );


        char getMoreButtonPresent();
        
        // 0 if not present
        double getMoreButtonExtent();
    };



#endif

        
