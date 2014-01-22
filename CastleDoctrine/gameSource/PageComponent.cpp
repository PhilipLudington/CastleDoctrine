#include "PageComponent.h"

#include "minorGems/game/game.h"


PageComponent::PageComponent( double inX, double inY )
        : mX( inX ), mY( inY ), mParent( NULL ), mVisible( true ) {
    
    }
        


doublePair PageComponent::getCenter() {
    doublePair c = { mX, mY };

    return c;
    }



void PageComponent::setParent( PageComponent *inParent ) {
    mParent = inParent;
    }



void PageComponent::setToolTip( const char *inTip ) {
    if( mParent != NULL ) {
        mParent->setToolTip( inTip );
        }
    }



void PageComponent::base_step(){
    for( int i=0; i<mComponents.size(); i++ ) {
        PageComponent *c = *( mComponents.getElement( i ) );
        
        if( c->isVisible() && c->isActive() ) {
            c->base_step();
            }
        }
    
    step();
    }



void PageComponent::base_draw( doublePair inViewCenter, 
                               double inViewSize ){

    doublePair oldViewCenter = getViewCenterPosition();

    setViewCenterPosition( oldViewCenter.x - mX, 
                           oldViewCenter.y - mY );
    
    for( int i=0; i<mComponents.size(); i++ ) {
        PageComponent *c = *( mComponents.getElement( i ) );
    
        if( c->isVisible() ) {
            c->base_draw( inViewCenter, inViewSize );
            }
        }

    draw();

    setViewCenterPosition( oldViewCenter.x, oldViewCenter.y );
    }



void PageComponent::setPosition( double inX, double inY ) {
    mX = inX;
    mY = inY;
    }



doublePair PageComponent::getPosition() {
    doublePair p;
    p.x = mX;
    p.y = mY;
    return p;
    }



void PageComponent::base_clearState(){
    
    for( int i=0; i<mComponents.size(); i++ ) {
        PageComponent *c = *( mComponents.getElement( i ) );

        c->base_clearState();
        }


    clearState();
    }



void PageComponent::base_pointerMove( float inX, float inY ){
    inX -= mX;
    inY -= mY;
    
    for( int i=0; i<mComponents.size(); i++ ) {
        PageComponent *c = *( mComponents.getElement( i ) );
    
        if( c->isVisible() && c->isActive() ) {
            c->base_pointerMove( inX, inY );
            }
        }
    
    pointerMove( inX, inY );
    }



void PageComponent::base_pointerDown( float inX, float inY ){
    inX -= mX;
    inY -= mY;
    
    for( int i=0; i<mComponents.size(); i++ ) {
        PageComponent *c = *( mComponents.getElement( i ) );
    
        if( c->isVisible() && c->isActive() ) {
            c->base_pointerDown( inX, inY );
            }
        }
    
    pointerDown( inX, inY );
    }



void PageComponent::base_pointerDrag( float inX, float inY ){
    inX -= mX;
    inY -= mY;
    
    for( int i=0; i<mComponents.size(); i++ ) {
        PageComponent *c = *( mComponents.getElement( i ) );
    
        if( c->isVisible() && c->isActive() ) {
            c->base_pointerDrag( inX, inY );
            }
        }

    pointerDrag( inX, inY );
    }



void PageComponent::base_pointerUp( float inX, float inY ){
    inX -= mX;
    inY -= mY;
    
    for( int i=0; i<mComponents.size(); i++ ) {
        PageComponent *c = *( mComponents.getElement( i ) );
    
        if( c->isVisible() && c->isActive() ) {
            c->base_pointerUp( inX, inY );
            }
        }

    pointerUp( inX, inY );
    }



void PageComponent::base_keyDown( unsigned char inASCII ){
    for( int i=0; i<mComponents.size(); i++ ) {
        PageComponent *c = *( mComponents.getElement( i ) );
    
        if( c->isVisible() && c->isActive() ) {
            c->base_keyDown( inASCII );
            }
        }

    keyDown( inASCII );
    }


        
void PageComponent::base_keyUp( unsigned char inASCII ){
    for( int i=0; i<mComponents.size(); i++ ) {
        PageComponent *c = *( mComponents.getElement( i ) );
    
        if( c->isVisible() && c->isActive() ) {
            c->base_keyUp( inASCII );
            }
        }

    keyUp( inASCII );
    }



void PageComponent::base_specialKeyDown( int inKeyCode ){
    for( int i=0; i<mComponents.size(); i++ ) {
        PageComponent *c = *( mComponents.getElement( i ) );
    
        if( c->isVisible() && c->isActive() ) {
            c->base_specialKeyDown( inKeyCode );
            }
        }

    specialKeyDown( inKeyCode );
    }



void PageComponent::base_specialKeyUp( int inKeyCode ){
    for( int i=0; i<mComponents.size(); i++ ) {
        PageComponent *c = *( mComponents.getElement( i ) );
    
        if( c->isVisible() && c->isActive() ) {
            c->base_specialKeyUp( inKeyCode );
            }
        }

    specialKeyUp( inKeyCode );
    }



void PageComponent::addComponent( PageComponent *inComponent ){

    mComponents.push_back( inComponent );

    inComponent->setParent( this );
    }



void PageComponent::setWaiting( char inWaiting ) {
    // pass up chain (stops at GamePage)
    if( mParent != NULL ) {
        mParent->setWaiting( inWaiting );
        }
    }
