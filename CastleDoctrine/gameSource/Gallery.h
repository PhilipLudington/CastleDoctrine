#ifndef GALLERY_INCLUDED
#define GALLERY_INCLUDED

#include "PageComponent.h"

#include "GallerySlotButton.h"

#include "GalleryArchive.h"


#define NUM_GALLERY_SLOTS   3


class  Gallery : public PageComponent, public ActionListener {
    public:
        
        Gallery( Font *inDisplayFont, double inX, double inY );
        

        ~Gallery();

        void setAllowEdit( char inAllow );

        void setGalleryContents( const char *inGalleryContents );
        
        char *getGalleryContents();
        
        virtual void draw();
        
        void actionPerformed( GUIComponent *inTarget );

        // toggles fadeOut or fadeIn status (triggering gradual fade change)
        void fadeOut( char inShouldFade );

        void instantFadeOut( char inShouldFade );
        
        virtual char isVisible();

    protected:

        GallerySlotButton *mGallerySlots[ NUM_GALLERY_SLOTS ];

        GalleryArchive mGalleryArchive;
        
        char mAllowEdit;
        
        float mFade;
        char mShouldFade;
        
    };



#endif

                        
