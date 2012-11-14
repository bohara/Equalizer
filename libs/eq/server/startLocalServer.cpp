
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
 *                    2010, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "global.h"
#include "loader.h"

#include <co/node.h>

#define CONFIG "server{ config{ appNode{ pipe {                            \
    window { viewport [ .25 .25 .5 .5 ] channel { name \"channel\" }}}}    \
    compound { channel \"channel\" wall { }}}}"

namespace
{
class ServerThread : public lunchbox::Thread
{
public:
    ServerThread() {}
    virtual ~ServerThread() {}

    bool start( eq::server::ServerPtr server )
        {
            LBASSERT( !_server );
            _server = server;
            return Thread::start();
        }
    
protected:

    virtual void run()
        {
            _server->run();
            _server->close();
            _server->deleteConfigs();

            LBINFO << "Server thread done, still referenced by " 
                   << _server->getRefCount() - 1 << std::endl;

            LBASSERTINFO( _server->getRefCount() == 1, _server );
            _server = 0;
            eq::server::Global::clear();
        }

private:
    eq::server::ServerPtr _server;    
};

static ServerThread _serverThread;
}
#pragma warning(push)
#pragma warning(disable: 4190)
extern "C" EQSERVER_API 
co::Connection* eqsStartLocalServer( const std::string& config )
{
#pragma warning(pop)
    if( _serverThread.isRunning( ))
    {
        LBWARN << "Only one app-local per process server allowed" << std::endl;
        return 0;
    }

    eq::server::Loader loader;
    eq::server::ServerPtr server;

    if( !config.empty() && config.find( ".eqc" ) == config.length() - 4 )
        server = loader.loadFile( config );
#ifdef EQ_USE_GPUSD
    else
        server = new eq::server::Server; // configured upon Server::chooseConfig
#endif

    if( !server )
        server = loader.parseServer( CONFIG );
    if( !server )
    {
        LBERROR << "Failed to load configuration" << std::endl;
        return 0;
    }

    eq::server::Loader::addOutputCompounds( server );
    eq::server::Loader::addDestinationViews( server );
    eq::server::Loader::addDefaultObserver( server );
    eq::server::Loader::convertTo11( server );
    eq::server::Loader::convertTo12( server );
    // TODO: ref count is 2 since config holds ServerPtr
    // LBASSERTINFO( server->getRefCount() == 1, server );

    co::ConnectionDescriptionPtr desc = new co::ConnectionDescription;
    desc->type = co::CONNECTIONTYPE_PIPE;
    co::ConnectionPtr connection = co::Connection::create( desc );
    if( !connection->connect( ))
    {
        LBERROR << "Failed to set up server connection" << std::endl;
        return 0;
    }

    co::ConnectionPtr sibling = connection->acceptSync();
    if( !server->listen( sibling ))
    {
        LBERROR << "Failed to setup server listener" << std::endl;
        return 0;
    }

    if( !_serverThread.start( server ))
    {
        LBERROR << "Failed to start server thread" << std::endl;
        return 0;
    }

    connection->ref(); // WAR "C" linkage
    return connection.get();
}

extern "C" EQSERVER_API void eqsJoinLocalServer()
{
    _serverThread.join();
}
