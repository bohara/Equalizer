
/* Copyright (c) 2010-2012, Stefan Eilemann <eile@eyescale.ch> 
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

#include "server.h"

#include "configPackets.h"
#include "configVisitor.h"
#include "elementVisitor.h"
#include "leafVisitor.h"
#include "log.h"
#include "serverPackets.h"

#include <co/command.h>
#include <co/connectionDescription.h>
#include <co/global.h>

namespace eq
{
namespace fabric
{

#define CmdFunc co::CommandFunc< Server< CL, S, CFG, NF, N, V > >

template< class CL, class S, class CFG, class NF, class N, class V >
Server< CL, S, CFG, NF, N, V >::Server( NF* nodeFactory )
        : _nodeFactory( nodeFactory )
{
    LBASSERT( nodeFactory );
    LBLOG( LOG_INIT ) << "New " << lunchbox::className( this ) << std::endl;
}

template< class CL, class S, class CFG, class NF, class N, class V >
Server< CL, S, CFG, NF, N, V >::~Server()
{
    LBLOG( LOG_INIT ) << "Delete " << lunchbox::className( this ) << std::endl;
    _client = 0;
    LBASSERT( _configs.empty( ));
}

template< class CL, class S, class CFG, class NF, class N, class V >
void Server< CL, S, CFG, NF, N, V >::setClient( ClientPtr client )
{
    _client = client;
    if( !client )
        return;

    co::CommandQueue* queue = static_cast< S* >( this )->getMainThreadQueue();
    this->registerCommand( CMD_SERVER_CREATE_CONFIG,
                     CmdFunc( this, &Server::_cmdCreateConfig ), queue );
    this->registerCommand( CMD_SERVER_DESTROY_CONFIG,
                     CmdFunc( this, &Server::_cmdDestroyConfig ), queue );
}

template< class CL, class S, class CFG, class NF, class N, class V >
void Server< CL, S, CFG, NF, N, V >::_addConfig( CFG* config )
{ 
    LBASSERT( config->getServer() == static_cast< S* >( this ));
    LBASSERT( stde::find( _configs, config ) == _configs.end( ));
    _configs.push_back( config );
}

template< class CL, class S, class CFG, class NF, class N, class V >
bool Server< CL, S, CFG, NF, N, V >::_removeConfig( CFG* config )
{
    typename Configs::iterator i = stde::find( _configs, config );
    if( i == _configs.end( ))
        return false;

    _configs.erase( i );
    return true;
}

namespace
{
template< class S, class V >
VisitorResult _accept( S* server, V& visitor )
{
    VisitorResult result = visitor.visitPre( server );
    if( result != TRAVERSE_CONTINUE )
        return result;

    const typename S::Configs& configs = server->getConfigs();
    for( typename S::Configs::const_iterator i = configs.begin(); 
         i != configs.end(); ++i )
    {
        switch( (*i)->accept( visitor ))
        {
            case TRAVERSE_TERMINATE:
                return TRAVERSE_TERMINATE;

            case TRAVERSE_PRUNE:
                result = TRAVERSE_PRUNE;
                break;
                
            case TRAVERSE_CONTINUE:
            default:
                break;
        }
    }

    switch( visitor.visitPost( server ))
    {
        case TRAVERSE_TERMINATE:
            return TRAVERSE_TERMINATE;

        case TRAVERSE_PRUNE:
            return TRAVERSE_PRUNE;
                
        case TRAVERSE_CONTINUE:
        default:
            break;
    }

    return result;
}
}

template< class CL, class S, class CFG, class NF, class N, class V >
VisitorResult Server< CL, S, CFG, NF, N, V >::accept( V& visitor )
{
    return _accept( static_cast< S* >( this ), visitor );
}

template< class CL, class S, class CFG, class NF, class N, class V >
VisitorResult Server< CL, S, CFG, NF, N, V >::accept( V& visitor ) const
{
    return _accept( static_cast< const S* >( this ), visitor );
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
template< class CL, class S, class CFG, class NF, class N, class V > bool
Server< CL, S, CFG, NF, N, V >::_cmdCreateConfig( co::Command& command )
{
    const ServerCreateConfigPacket* packet = 
        command.get<ServerCreateConfigPacket>();
    LBVERB << "Handle create config " << packet << std::endl;
    CFG* config = _nodeFactory->createConfig( static_cast< S* >( this ));
    co::LocalNodePtr localNode = command.getLocalNode();
    localNode->mapObject( config, packet->configVersion );
    co::Global::setIAttribute( co::Global::IATTR_ROBUSTNESS, 
                               config->getIAttribute( CFG::IATTR_ROBUSTNESS ));
    if( packet->requestID != LB_UNDEFINED_UINT32 )
    {
        ConfigCreateReplyPacket reply( packet );
        command.getNode()->send( reply );
    }

    return true;
}

template< class CL, class S, class CFG, class NF, class N, class V > bool
Server< CL, S, CFG, NF, N, V >::_cmdDestroyConfig( co::Command& command )
{
    const ServerDestroyConfigPacket* packet = 
        command.get<ServerDestroyConfigPacket>();
    LBVERB << "Handle destroy config " << packet << std::endl;
    
    co::LocalNodePtr localNode = command.getLocalNode();

    CFG* config = 0;
    for( typename Configs::const_iterator i = _configs.begin();
         i != _configs.end(); ++i )
    {
        if( (*i)->getID() ==  packet->configID )
        {
            config = *i;
            break;
        }
    }
    LBASSERT( config );

    localNode->unmapObject( config );
    _nodeFactory->releaseConfig( config );

    if( packet->requestID != LB_UNDEFINED_UINT32 )
    {
        ServerDestroyConfigReplyPacket reply( packet );
        command.getNode()->send( reply );
    }
    return true;
}

template< class CL, class S, class CFG, class NF, class N, class V >
std::ostream& operator << ( std::ostream& os, 
                            const Server< CL, S, CFG, NF, N, V >& server )
{
    os << lunchbox::disableFlush << lunchbox::disableHeader << "server "
       << std::endl;
    os << "{" << std::endl << lunchbox::indent;
    
    const co::ConnectionDescriptions& cds =
        server.getConnectionDescriptions();
    for( co::ConnectionDescriptions::const_iterator i = cds.begin();
         i != cds.end(); ++i )
    {
        co::ConnectionDescriptionPtr desc = *i;
        os << *desc;
    }

    const std::vector< CFG* >& configs = server.getConfigs();
    for( typename std::vector< CFG* >::const_iterator i = configs.begin();
         i != configs.end(); ++i )
    {
        const CFG* config = *i;
        os << *config;
    }

    os << lunchbox::exdent << "}"  << lunchbox::enableHeader 
       << lunchbox::enableFlush << std::endl;

    return os;
}

}
}
