
/* Copyright (c) 2009-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "observer.h"

#include "leafVisitor.h"
#include "log.h"
#include "paths.h"

#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace eq
{
namespace fabric
{

template< typename C, typename O >
Observer< C, O >::Observer( C* config )
        : _config( config )
{
    LBASSERT( config );
    config->_addObserver( static_cast< O* >( this ));
    _data.eyeBase = config->getFAttribute( C::FATTR_EYE_BASE );
    LBLOG( LOG_INIT ) << "New " << lunchbox::className( this ) << std::endl;
}

template< typename C, typename O >
Observer< C, O >::~Observer()
{
    LBLOG( LOG_INIT ) << "Delete " << lunchbox::className( this ) << std::endl;
    _config->_removeObserver( static_cast< O* >( this ));
}

template< typename C, typename O >
Observer< C, O >::BackupData::BackupData()
        : eyeBase( .05f )
        , focusDistance( 1.f )
        , focusMode( FOCUSMODE_FIXED )
        , headMatrix( Matrix4f::IDENTITY )
{}

template< typename C, typename O >
void Observer< C, O >::backup()
{
    Object::backup();
    _backup = _data;
}

template< typename C, typename O >
void Observer< C, O >::restore()
{
    _data = _backup;
    Object::restore();
    setDirty( DIRTY_EYE_BASE | DIRTY_HEAD | DIRTY_FOCUS );
}

template< typename C, typename O >
void Observer< C, O >::serialize( co::DataOStream& os,
                                  const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );

    if( dirtyBits & DIRTY_EYE_BASE )
        os << _data.eyeBase;
    if( dirtyBits & DIRTY_FOCUS )
        os << _data.focusDistance << _data.focusMode;
    if( dirtyBits & DIRTY_HEAD )
        os << _data.headMatrix;
}

template< typename C, typename O >
void Observer< C, O >::deserialize( co::DataIStream& is,
                                    const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );

    if( dirtyBits & DIRTY_EYE_BASE )
        is >> _data.eyeBase;
    if( dirtyBits & DIRTY_FOCUS )
        is >> _data.focusDistance >> _data.focusMode;
    if( dirtyBits & DIRTY_HEAD )
        is >> _data.headMatrix;
}

template< typename C, typename O >
void Observer< C, O >::setDirty( const uint64_t dirtyBits )
{
    Object::setDirty( dirtyBits );
    _config->setDirty( C::DIRTY_OBSERVERS );
}

template< typename C, typename O >
VisitorResult Observer< C, O >::accept( Visitor& visitor )
{
    return visitor.visit( static_cast< O* >( this ));
}

template< typename C, typename O >
VisitorResult Observer< C, O >::accept( Visitor& visitor ) const
{
    return visitor.visit( static_cast< const O* >( this ));
}

template< typename C, typename O >
ObserverPath Observer< C, O >::getPath() const
{
    const std::vector< O* >&  observers = _config->getObservers();
    typename std::vector< O* >::const_iterator i = std::find( observers.begin(),
                                                              observers.end(),
                                                              this );
    LBASSERT( i != observers.end( ));

    ObserverPath path;
    path.observerIndex = std::distance( observers.begin(), i );
    return path;
}

template< typename C, typename O >
void Observer< C, O >::setEyeBase( const float eyeBase )
{
    if( _data.eyeBase == eyeBase )
        return;

    _data.eyeBase = eyeBase;
    setDirty( DIRTY_EYE_BASE );
}

template< typename C, typename O >
void Observer< C, O >::setFocusDistance( const float focusDistance )
{
    if( _data.focusDistance == focusDistance )
        return;

    _data.focusDistance = focusDistance;
    setDirty( DIRTY_FOCUS );
}

template< typename C, typename O >
void Observer< C, O >::setFocusMode( const FocusMode focusMode )
{
    if( _data.focusMode == focusMode )
        return;

    _data.focusMode = focusMode;
    setDirty( DIRTY_FOCUS );
}

template< typename C, typename O >
void Observer< C, O >::setHeadMatrix( const Matrix4f& matrix )
{
    if( _data.headMatrix == matrix )
        return;

    _data.headMatrix = matrix;
    setDirty( DIRTY_HEAD );
}

template< typename C, typename O >
std::ostream& operator << ( std::ostream& os, const Observer< C, O >& observer )
{
    os << lunchbox::disableFlush << lunchbox::disableHeader << "observer"
       << std::endl;
    os << "{" << std::endl << lunchbox::indent; 

    const std::string& name = observer.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const float eyeBase = observer.getEyeBase();
    if( eyeBase != observer.getConfig()->getFAttribute( C::FATTR_EYE_BASE ))
        os << "eye_base       " << eyeBase << std::endl;

    const float focusDistance = observer.getFocusDistance();
    os << "focus_distance " << focusDistance << std::endl;

    const FocusMode focusMode = observer.getFocusMode();
    os << "focus_mode     " << focusMode << std::endl;

    os << lunchbox::exdent << "}" << std::endl << lunchbox::enableHeader
       << lunchbox::enableFlush;
    return os;
}

}
}
