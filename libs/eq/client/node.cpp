
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder<cedric.stalder@gmail.com>
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

#include "node.h"

#include "client.h"
#include "config.h"
#include "configPackets.h"
#include "error.h"
#include "exception.h"
#include "frameData.h"
#include "global.h"
#include "log.h"
#include "nodeFactory.h"
#include "nodePackets.h"
#include "nodeStatistics.h"
#include "pipe.h"
#include "pipePackets.h"
#include "server.h"

#include <eq/fabric/elementVisitor.h>
#include <eq/fabric/packets.h>
#include <eq/fabric/task.h>
#include <co/barrier.h>
#include <co/command.h>
#include <co/connection.h>
#include <lunchbox/scopedMutex.h>

namespace eq
{
/** @cond IGNORE */
typedef co::CommandFunc<Node> NodeFunc;
typedef fabric::Node< Config, Node, Pipe, NodeVisitor > Super;
/** @endcond */

Node::Node( Config* parent )
        : Super( parent )
#pragma warning(push)
#pragma warning(disable: 4355)
        , transmitter( this )
#pragma warning(pop)
        , _state( STATE_STOPPED )
        , _finishedFrame( 0 )
        , _unlockedFrame( 0 )
{
}

Node::~Node()
{
    LBASSERT( getPipes().empty( ));
}

void Node::attach( const UUID& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );

    co::CommandQueue* queue = getMainThreadQueue();
    co::CommandQueue* commandQ = getCommandThreadQueue();
    co::CommandQueue* transmitQ = &transmitter.getQueue();

    registerCommand( fabric::CMD_NODE_CREATE_PIPE,
                     NodeFunc( this, &Node::_cmdCreatePipe ), queue );
    registerCommand( fabric::CMD_NODE_DESTROY_PIPE,
                     NodeFunc( this, &Node::_cmdDestroyPipe ), queue );
    registerCommand( fabric::CMD_NODE_CONFIG_INIT, 
                     NodeFunc( this, &Node::_cmdConfigInit ), queue );
    registerCommand( fabric::CMD_NODE_SET_AFFINITY,
                     NodeFunc( this, &Node::_cmdSetAffinity), transmitQ );
    registerCommand( fabric::CMD_NODE_CONFIG_EXIT,
                     NodeFunc( this, &Node::_cmdConfigExit ), queue );
    registerCommand( fabric::CMD_NODE_FRAME_START,
                     NodeFunc( this, &Node::_cmdFrameStart ), queue );
    registerCommand( fabric::CMD_NODE_FRAME_FINISH,
                     NodeFunc( this, &Node::_cmdFrameFinish ), queue );
    registerCommand( fabric::CMD_NODE_FRAME_DRAW_FINISH, 
                     NodeFunc( this, &Node::_cmdFrameDrawFinish ), queue );
    registerCommand( fabric::CMD_NODE_FRAME_TASKS_FINISH, 
                     NodeFunc( this, &Node::_cmdFrameTasksFinish ), queue );
    registerCommand( fabric::CMD_NODE_FRAMEDATA_TRANSMIT,
                     NodeFunc( this, &Node::_cmdFrameDataTransmit ), commandQ );
    registerCommand( fabric::CMD_NODE_FRAMEDATA_READY,
                     NodeFunc( this, &Node::_cmdFrameDataReady ), commandQ );
}

void Node::setDirty( const uint64_t bits )
{
    // jump over fabric setDirty to avoid dirty'ing config node list
    // nodes are individually synced in frame finish
    Object::setDirty( bits );
}

ClientPtr Node::getClient()
{
    Config* config = getConfig();
    LBASSERT( config );
    return ( config ? config->getClient() : 0 );
}

ServerPtr Node::getServer()
{
    Config* config = getConfig();
    LBASSERT( config );
    return ( config ? config->getServer() : 0 );
}

co::CommandQueue* Node::getMainThreadQueue()
{
    return getConfig()->getMainThreadQueue();
}

co::CommandQueue* Node::getCommandThreadQueue()
{
    return getConfig()->getCommandThreadQueue();
}

co::Barrier* Node::getBarrier( const co::ObjectVersion barrier )
{
    lunchbox::ScopedMutex<> mutex( _barriers );
    co::Barrier* netBarrier = _barriers.data[ barrier.identifier ];

    if( netBarrier )
        netBarrier->sync( barrier.version );
    else
    {
        ClientPtr client = getClient();

        netBarrier = new co::Barrier;
        LBCHECK( client->mapObject( netBarrier, barrier ));

        _barriers.data[ barrier.identifier ] = netBarrier;
    }

    return netBarrier;
}

FrameDataPtr Node::getFrameData( const co::ObjectVersion& frameDataVersion )
{
    lunchbox::ScopedWrite mutex( _frameDatas );
    FrameDataPtr data = _frameDatas.data[ frameDataVersion.identifier ];

    if( !data )
    {
        data = new FrameData;
        data->setID( frameDataVersion.identifier );
        _frameDatas.data[ frameDataVersion.identifier ] = data;
    }

    LBASSERT( frameDataVersion.version.high() == 0 );
    data->setVersion( frameDataVersion.version.low( ));
    return data;
}

void Node::releaseFrameData( FrameDataPtr data )
{
    lunchbox::ScopedWrite mutex( _frameDatas );
    FrameDataHashIter i = _frameDatas->find( data->getID( ));
    LBASSERT( i != _frameDatas->end( ));
    if( i == _frameDatas->end( ))
        return;

    _frameDatas->erase( i );
}

void Node::waitInitialized() const
{
    _state.waitGE( STATE_INIT_FAILED );
}

bool Node::isRunning() const
{
    return (_state == STATE_RUNNING);
}

bool Node::isStopped() const
{
    return (_state == STATE_STOPPED);
}

bool Node::configInit( const uint128_t& )
{
    WindowSystem::configInit( this );
    return true;
}

bool Node::configExit()
{
    WindowSystem::configExit( this );
    return true;
}

void Node::_setAffinity()
{
    const int32_t affinity = getIAttribute( IATTR_HINT_AFFINITY );
    switch( affinity )
    {
        case OFF:
            break;

        case AUTO:
            // TODO
            LBVERB << "No automatic thread placement for node threads "
                   << std::endl;
            break;

        default:
            NodeAffinityPacket packet;
            packet.affinity = affinity;
            co::LocalNodePtr node = getLocalNode();
            send( node, packet );

            node->setAffinity( affinity );
            break;
    }
}

void Node::waitFrameStarted( const uint32_t frameNumber ) const
{
    _currentFrame.waitGE( frameNumber );
}

void Node::startFrame( const uint32_t frameNumber ) 
{
    _currentFrame = frameNumber;
}

void Node::frameFinish( const uint128_t&, const uint32_t frameNumber ) 
{
    releaseFrame( frameNumber );
}

void Node::_finishFrame( const uint32_t frameNumber ) const
{
    const Pipes& pipes = getPipes();
    for( Pipes::const_iterator i = pipes.begin(); i != pipes.end(); ++i )
    {
        const Pipe* pipe = *i;
        LBASSERT( pipe->isThreaded() || pipe->getFinishedFrame()>=frameNumber );

        pipe->waitFrameLocal( frameNumber );
        pipe->waitFrameFinished( frameNumber );
    }
}

void Node::_frameFinish( const uint128_t& frameID, 
                         const uint32_t frameNumber )
{
    frameFinish( frameID, frameNumber );
    LBLOG( LOG_TASKS ) << "---- Finished Frame --- " << frameNumber
                       << std::endl;

    if( _unlockedFrame < frameNumber )
    {
        LBWARN << "Finished frame was not locally unlocked, enforcing unlock" 
               << std::endl;
        releaseFrameLocal( frameNumber );
    }

    if( _finishedFrame < frameNumber )
    {
        LBWARN << "Finished frame was not released, enforcing unlock"
               << std::endl;
        releaseFrame( frameNumber );
    }
}

void Node::releaseFrame( const uint32_t frameNumber )
{
    LBASSERTINFO( _currentFrame >= frameNumber, 
                  "current " << _currentFrame << " release " << frameNumber );

    if( _finishedFrame >= frameNumber )
        return;
    _finishedFrame = frameNumber;

    NodeFrameFinishReplyPacket packet;
    packet.frameNumber = frameNumber;

    Config* config = getConfig();
    ServerPtr server = config->getServer();
    co::NodePtr node = server.get();
    send( node, packet );
}

void Node::releaseFrameLocal( const uint32_t frameNumber )
{
    LBASSERT( _unlockedFrame <= frameNumber );
    _unlockedFrame = frameNumber;
    
    Config* config = getConfig();
    LBASSERT( config->getNodes().size() == 1 );
    LBASSERT( config->getNodes()[0] == this );
    config->releaseFrameLocal( frameNumber );

    LBLOG( LOG_TASKS ) << "---- Unlocked Frame --- " << _unlockedFrame
                       << std::endl;
}

void Node::frameStart( const uint128_t&, const uint32_t frameNumber )
{
    startFrame( frameNumber ); // unlock pipe threads
    
    switch( getIAttribute( IATTR_THREAD_MODEL ))
    {
        case ASYNC:
            // Don't wait for pipes to release frame locally, sync not needed
            releaseFrameLocal( frameNumber );
            break;

        case DRAW_SYNC:  // Sync and release in frameDrawFinish
        case LOCAL_SYNC: // Sync and release in frameTasksFinish
            break;

        default:
            LBUNIMPLEMENTED;
    }
}

void Node::frameDrawFinish( const uint128_t&, const uint32_t frameNumber )
{
    switch( getIAttribute( IATTR_THREAD_MODEL ))
    {
        case ASYNC:      // No sync, release in frameStart
        case LOCAL_SYNC: // Sync and release in frameTasksFinish
            break;

        case DRAW_SYNC:
        {
            const Pipes& pipes = getPipes();
            for( Pipes::const_iterator i = pipes.begin();
                 i != pipes.end(); ++i )
            {
                const Pipe* pipe = *i;
                if( pipe->getTasks() & fabric::TASK_DRAW )
                    pipe->waitFrameLocal( frameNumber );
            }
            
            releaseFrameLocal( frameNumber );
            break;
        }
        default:
            LBUNIMPLEMENTED;
    }
}

void Node::frameTasksFinish( const uint128_t&, const uint32_t frameNumber )
{
    switch( getIAttribute( IATTR_THREAD_MODEL ))
    {
        case ASYNC:      // No sync, release in frameStart
        case DRAW_SYNC:  // Sync and release in frameDrawFinish
            break;

        case LOCAL_SYNC:
        {
            const Pipes& pipes = getPipes();
            for( Pipes::const_iterator i = pipes.begin(); i != pipes.end(); ++i)
            {
                const Pipe* pipe = *i;
                if( pipe->getTasks() != fabric::TASK_NONE )
                    pipe->waitFrameLocal( frameNumber );
            }
            
            releaseFrameLocal( frameNumber );
            break;
        }
        default:
            LBUNIMPLEMENTED;
    }
}

void Node::_flushObjects()
{
    ClientPtr client = getClient();
    {
        lunchbox::ScopedMutex<> mutex( _barriers );
        for( BarrierHash::const_iterator i =_barriers->begin();
             i != _barriers->end(); ++ i )
        {
            co::Barrier* barrier = i->second;
            client->unmapObject( barrier );
            delete barrier;
        }
        _barriers->clear();
    }

    lunchbox::ScopedMutex<> mutex( _frameDatas );
    for( FrameDataHashCIter i = _frameDatas->begin();
         i != _frameDatas->end(); ++i )
    {
        FrameDataPtr frameData = i->second;
        frameData->resetPlugins();
        client->unmapObject( frameData.get( ));
    }
    _frameDatas->clear();
}

void Node::TransmitThread::run()
{
    lunchbox::Thread::setName( std::string( "Trm " ) +
                               lunchbox::className( _node ));
    while( true )
    {
        co::CommandPtr command = _queue.pop();
        if( !command )
            return; // exit thread

        LBCHECK( (*command)( ));
    }
}

void Node::dirtyClientExit()
{
    const Pipes& pipes = getPipes();
    for( PipesCIter i = pipes.begin(); i != pipes.end(); ++i )
    {
        Pipe* pipe = *i;
        pipe->cancelThread();
    }
    transmitter.getQueue().push( 0 ); // wake up to exit
    transmitter.join();
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool Node::_cmdCreatePipe( co::Command& command )
{
    const NodeCreatePipePacket* packet = 
        command.get<NodeCreatePipePacket>();
    LBLOG( LOG_INIT ) << "Create pipe " << packet << std::endl;
    LB_TS_THREAD( _nodeThread );
    LBASSERT( _state >= STATE_INIT_FAILED );

    Pipe* pipe = Global::getNodeFactory()->createPipe( this );
    if( packet->threaded )
        pipe->startThread();

    Config* config = getConfig();
    LBCHECK( config->mapObject( pipe, packet->pipeID ));
    pipe->notifyMapped();

    return true;
}

bool Node::_cmdDestroyPipe( co::Command& command )
{
    LB_TS_THREAD( _nodeThread );

    const NodeDestroyPipePacket* packet = 
        command.get< NodeDestroyPipePacket >();
    LBLOG( LOG_INIT ) << "Destroy pipe " << packet << std::endl;

    Pipe* pipe = findPipe( packet->pipeID );
    LBASSERT( pipe );
    pipe->exitThread();

    PipeConfigExitReplyPacket reply( packet->pipeID, pipe->isStopped( ));

    Config* config = getConfig();
    config->unmapObject( pipe );
    Global::getNodeFactory()->releasePipe( pipe );

    getServer()->send( reply ); // send to config object
    return true;
}

bool Node::_cmdConfigInit( co::Command& command )
{
    LB_TS_THREAD( _nodeThread );

    const NodeConfigInitPacket* packet = 
        command.get<NodeConfigInitPacket>();
    LBLOG( LOG_INIT ) << "Init node " << packet << std::endl;

    _state = STATE_INITIALIZING;

    _currentFrame  = packet->frameNumber;
    _unlockedFrame = packet->frameNumber;
    _finishedFrame = packet->frameNumber;
    _setAffinity();

    transmitter.start();
    setError( ERROR_NONE );
    NodeConfigInitReplyPacket reply;
    reply.result = configInit( packet->initID );

    if( getIAttribute( IATTR_THREAD_MODEL ) == eq::UNDEFINED )
        setIAttribute( IATTR_THREAD_MODEL, eq::DRAW_SYNC );

    _state = reply.result ? STATE_RUNNING : STATE_INIT_FAILED;

    commit();
    send( command.getNode(), reply );
    return true;
}

bool Node::_cmdConfigExit( co::Command& command )
{
    LB_TS_THREAD( _nodeThread );
    LBLOG( LOG_INIT ) << "Node exit " 
                      << command.get<NodeConfigExitPacket>() << std::endl;

    const Pipes& pipes = getPipes();
    for( Pipes::const_iterator i = pipes.begin(); i != pipes.end(); ++i )
    {
        Pipe* pipe = *i;
        pipe->waitExited();
    }
    
    _state = configExit() ? STATE_STOPPED : STATE_FAILED;
    transmitter.getQueue().push( 0 ); // wake up to exit
    transmitter.join();
    _flushObjects();

    ConfigDestroyNodePacket destroyPacket( getID( ));
    getConfig()->send( getLocalNode(), destroyPacket );
    return true;
}

bool Node::_cmdFrameStart( co::Command& command )
{
    LB_TS_THREAD( _nodeThread );
    const NodeFrameStartPacket* packet = 
        command.get<NodeFrameStartPacket>();
    LBVERB << "handle node frame start " << packet << std::endl;

    const uint32_t frameNumber = packet->frameNumber;
    LBASSERT( _currentFrame == frameNumber-1 );

    LBLOG( LOG_TASKS ) << "----- Begin Frame ----- " << frameNumber
                       << std::endl;

    Config* config = getConfig();
    
    if( packet->configVersion != co::VERSION_INVALID )
        config->sync( packet->configVersion );
    sync( packet->version );

    config->_frameStart();
    frameStart( packet->frameID, frameNumber );

    LBASSERTINFO( _currentFrame >= frameNumber, 
                  "Node::frameStart() did not start frame " << frameNumber );
    return true;
}

bool Node::_cmdFrameFinish( co::Command& command )
{
    LB_TS_THREAD( _nodeThread );
    const NodeFrameFinishPacket* packet = 
        command.get<NodeFrameFinishPacket>();
    LBLOG( LOG_TASKS ) << "TASK frame finish " << getName() <<  " " << packet
                       << std::endl;

    const uint32_t frameNumber = packet->frameNumber;

    _finishFrame( frameNumber );
    _frameFinish( packet->frameID, frameNumber );

    const uint128_t version = commit();
    if( version != co::VERSION_NONE )
    {
        fabric::ObjectSyncPacket syncPacket;
        send( command.getNode(), syncPacket );
    }
    return true;
}

bool Node::_cmdFrameDrawFinish( co::Command& command )
{
    const NodeFrameDrawFinishPacket* packet = 
        command.get< NodeFrameDrawFinishPacket >();
    LBLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << packet
                       << std::endl;

    frameDrawFinish( packet->frameID, packet->frameNumber );
    return true;
}

bool Node::_cmdFrameTasksFinish( co::Command& command )
{
    const NodeFrameTasksFinishPacket* packet = 
        command.get< NodeFrameTasksFinishPacket >();
    LBLOG( LOG_TASKS ) << "TASK tasks finish " << getName() <<  " " << packet
                       << std::endl;

    frameTasksFinish( packet->frameID, packet->frameNumber );
    return true;
}

bool Node::_cmdFrameDataTransmit( co::Command& command )
{
    const NodeFrameDataTransmitPacket* packet =
        command.get<NodeFrameDataTransmitPacket>();

    LBLOG( LOG_ASSEMBLY )
        << "received image data for " << packet->frameData << ", buffers "
        << packet->buffers << " pvp " << packet->pvp << std::endl;

    LBASSERT( packet->pvp.isValid( ));

    FrameDataPtr frameData = getFrameData( packet->frameData );
    LBASSERT( !frameData->isReady() );

    NodeStatistics event( Statistic::NODE_FRAME_DECOMPRESS, this,
                          packet->frameNumber );
    LBCHECK( frameData->addImage( packet ));
    return true;
}

bool Node::_cmdFrameDataReady( co::Command& command )
{
    const NodeFrameDataReadyPacket* packet =
        command.get<NodeFrameDataReadyPacket>();

    LBLOG( LOG_ASSEMBLY ) << "received ready for " << packet->frameData
                          << std::endl;
    FrameDataPtr frameData = getFrameData( packet->frameData );
    LBASSERT( frameData );
    LBASSERT( !frameData->isReady() );
    frameData->setReady( packet );
    LBASSERT( frameData->isReady() );
    return true;
}

bool Node::_cmdSetAffinity( co::Command& command )
{
    const NodeAffinityPacket* packet = command.get <NodeAffinityPacket>();
    lunchbox::Thread::setAffinity( packet->affinity );
    return true;
}
}

#include "../fabric/node.ipp"
template class eq::fabric::Node< eq::Config, eq::Node, eq::Pipe,
                                 eq::NodeVisitor >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                             const eq::Super& );
/** @endcond */
