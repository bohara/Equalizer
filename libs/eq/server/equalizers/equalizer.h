
/* Copyright (c) 2008-2011, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef EQSERVER_EQUALIZER_H
#define EQSERVER_EQUALIZER_H

#include "../api.h"
#include "../compoundListener.h" // base class
#include "../types.h"
#include <eq/fabric/equalizerTypes.h>

namespace eq
{
namespace server
{
    /**
     * A generic equalizer interface.
     *
     * An equalizer is attached to a compound tree, on which it balances render
     * tasks. It can update its compound tree on the beginnning of each
     * frame. It has to subscribe to the statistics events needed to perform its
     * tasks itself.
     */
    class Equalizer : protected CompoundListener
    {
    public:
        Equalizer();
        Equalizer( const Equalizer& from );
        virtual ~Equalizer();


        /** Output to a stream. */
        virtual void toStream( std::ostream& os ) const = 0;

        /** @return the compound attached to. */
        const Compound* getCompound() const { return _compound; }
        Compound* getCompound()             { return _compound; }

        /** @return the config. */
        const Config* getConfig() const;

        /** Attach to a compound and detach the previous compound. */
        virtual void attach( Compound* compound );

        void setFrozen( const bool onOff ) { _frozen = onOff; }
        bool isFrozen() const { return _frozen; }

        virtual void setActive( bool flag ) { _active = flag; }
        virtual bool isActive() const { return _active; }

        virtual uint32_t getType() const = 0;

    private:
        // override in sub-classes to handle dynamic compounds.
        virtual void notifyChildAdded( Compound* compound, Compound* child )
            { LBUNIMPLEMENTED }
        virtual void notifyChildRemove( Compound* compound, Compound* child )
            { LBUNIMPLEMENTED }

        Compound* _compound;       //!< The attached compound
        bool      _frozen;
        bool      _active;
    };

    inline std::ostream& operator << ( std::ostream& os, const Equalizer* eq )
    {
        if( eq )
            eq->toStream( os );
        return os;
    }
}
}

#endif // EQSERVER_EQUALIZER_H
