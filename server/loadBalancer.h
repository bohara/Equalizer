
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_LOADBALANCER_H
#define EQS_LOADBALANCER_H

#include "compoundListener.h" // base class
#include "types.h"

#include <eq/client/range.h>
#include <eq/client/viewport.h>

namespace eq
{
namespace server
{
    class LoadBalancer;

    /** The interface to the concrete loadbalancer implementation. */
    class LoadBalancerIF
    {
    public:
        LoadBalancerIF( const LoadBalancer& parent ) : _parent( parent ) {}
        virtual ~LoadBalancerIF() {}

        virtual void update( const uint32_t frameNumber ) = 0;

    protected:
        const LoadBalancer& _parent;
    };

    /**
     * A generic loadbalancer interface using different implementations to adapt
     * compounds based on runtime data.
     */
    class LoadBalancer : protected CompoundListener
    {
    public:
        LoadBalancer();
        LoadBalancer( const LoadBalancer& from );
        virtual ~LoadBalancer();

        enum Mode
        {
            MODE_DB = 0,     //!< Adapt for a sort-last decomposition
            MODE_HORIZONTAL, //!< Adapt for sort-first using horizontal stripes
            MODE_VERTICAL,   //!< Adapt for sort-first using vertical stripes
            MODE_2D,         //!< Adapt for a sort-first decomposition
            MODE_DPLEX,      //!< Adapt for smooth time-multiplex rendering
            MODE_DFR         //!< Adapt for dynamic frame zoom rendering
        };

        /** Set the load balancer adaptation mode. */
        void setMode( const Mode mode ) { _mode = mode; }

        /** Undocumented  */
        void setDamping( const float damping ) { _damping = damping; }
        float getDamping() const { return _damping; }

        /** @return the load balancer adaptation mode. */
        Mode getMode() const { return _mode; }

        /** @return the currently attached compound. */
        Compound* getCompound() const { return _compound; }

        /** Attach to a compound and detach the previous compound. */
        void attach( Compound* compound );

        /** @sa CompoundListener::notifyUpdatePre */
        virtual void notifyUpdatePre( Compound* compound, 
                                      const uint32_t frameNumber );

        /** @sa CompoundListener::notifyChildAdded */
        virtual void notifyChildAdded( Compound* compound, Compound* child );

        /** @sa CompoundListener::notifyChildRemove */
        virtual void notifyChildRemove( Compound* compound, Compound* child );

        void setFreeze( const bool onOff ) { _freeze = onOff; }
        bool isFrozen() const { return _freeze; }

        /** Set the average Frame Rate using in dfrLoadBalancer  */
        void setFrameRate( const float frameRate ) { _frameRate = frameRate; }

        /** Get the average Frame Rate using in dfrLoadBalancer  */ 
        float getFrameRate() const{ return _frameRate; }
        
    private:
        //-------------------- Members --------------------
        Mode  _mode;    //!< The current adaptation mode
        float _damping; //!< The damping factor,  (0: No damping, 1: No changes)
        float _frameRate; // the fps setting
        
        Compound*       _compound;       //!< The attached compound
        LoadBalancerIF* _implementation; //!< The concrete implementation

        bool _freeze;
        
        void _clear();
    };

    std::ostream& operator << ( std::ostream& os, const LoadBalancer* lb );
    std::ostream& operator << ( std::ostream& os, const LoadBalancer::Mode );
}
}

#endif // EQS_LOADBALANCER_H
