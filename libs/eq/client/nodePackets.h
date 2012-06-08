
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder  <cedric.stalder@gmail.com>
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

#ifndef EQ_NODEPACKETS_H
#define EQ_NODEPACKETS_H

#include <eq/client/packets.h>   // base structs
#include <eq/client/frameData.h> // member

/** @cond IGNORE */
namespace eq
{

    struct NodeConfigInitPacket : public co::ObjectPacket
    {
        NodeConfigInitPacket()
            {
                command        = fabric::CMD_NODE_CONFIG_INIT;
                size           = sizeof( NodeConfigInitPacket );
            }

        uint128_t initID;
        uint32_t frameNumber;
    };

    struct NodeConfigInitReplyPacket : public co::ObjectPacket
    {
        NodeConfigInitReplyPacket()
            {
                command   = fabric::CMD_NODE_CONFIG_INIT_REPLY;
                size      = sizeof( NodeConfigInitReplyPacket );
            }

        bool     result;
    };

    struct NodeConfigExitPacket : public NodePacket
    {
        NodeConfigExitPacket()
            {
                command = fabric::CMD_NODE_CONFIG_EXIT;
                size    = sizeof( NodeConfigExitPacket );
            }
    };

    struct NodeConfigExitReplyPacket : public NodePacket
    {
        NodeConfigExitReplyPacket( const UUID& nodeID, const bool res )
                : result( res )
            {
                command   = fabric::CMD_NODE_CONFIG_EXIT_REPLY;
                size      = sizeof( NodeConfigExitReplyPacket );
                objectID  = nodeID;
            }

        const bool result;
    };

    struct NodeCreatePipePacket : public NodePacket
    {
        NodeCreatePipePacket()
            {
                command = fabric::CMD_NODE_CREATE_PIPE;
                size    = sizeof( NodeCreatePipePacket );
            }

        UUID pipeID;
        bool     threaded;
    };

    struct NodeDestroyPipePacket : public NodePacket
    {
        NodeDestroyPipePacket( const UUID& id )
                : pipeID( id )
            {
                command = fabric::CMD_NODE_DESTROY_PIPE;
                size    = sizeof( NodeDestroyPipePacket );
            }

        const UUID pipeID;
    };
    
    struct NodeFrameStartPacket : public NodePacket
    {
        NodeFrameStartPacket() : configVersion( co::VERSION_INVALID )
            {
                command        = fabric::CMD_NODE_FRAME_START;
                size           = sizeof( NodeFrameStartPacket );
            }

        uint128_t version;
        uint128_t configVersion;
        uint128_t frameID;
        uint32_t frameNumber;
    };

    struct NodeFrameFinishPacket : public NodePacket
    {
        NodeFrameFinishPacket()
            {
                command          = fabric::CMD_NODE_FRAME_FINISH;
                size             = sizeof( NodeFrameFinishPacket );
            }

        uint128_t frameID;
        uint32_t frameNumber;
    };

    struct NodeFrameFinishReplyPacket : public NodePacket
    {
        NodeFrameFinishReplyPacket()
            {
                command        = fabric::CMD_NODE_FRAME_FINISH_REPLY;
                size           = sizeof( NodeFrameFinishReplyPacket );
            }

        uint32_t frameNumber;
    };
        
    struct NodeFrameDrawFinishPacket : public NodePacket
    {
        NodeFrameDrawFinishPacket()
            {
                command       = fabric::CMD_NODE_FRAME_DRAW_FINISH;
                size          = sizeof( NodeFrameDrawFinishPacket );
            }
        uint128_t frameID;
        uint32_t frameNumber;
    };

    struct NodeFrameDataTransmitPacket : public NodePacket
    {
        NodeFrameDataTransmitPacket()
            {
                command = fabric::CMD_NODE_FRAMEDATA_TRANSMIT;
                size    = sizeof( NodeFrameDataTransmitPacket );
            }

        co::ObjectVersion frameData;
        PixelViewport pvp;
        Zoom          zoom;
        uint32_t      buffers;
        uint32_t      frameNumber;
        uint64_t useAlpha; // bool + valgrind padding

        LB_ALIGN8( uint8_t data[8] );
    };

    struct NodeFrameDataReadyPacket : public NodePacket
    {
        NodeFrameDataReadyPacket( const FrameDataPtr fd )
                : frameData( fd ), data( fd->_data )
            {
                command = fabric::CMD_NODE_FRAMEDATA_READY;
                size    = sizeof( NodeFrameDataReadyPacket );
            }

        const co::ObjectVersion frameData;
        const FrameData::Data data;
    };

    struct NodeFrameTasksFinishPacket : public NodePacket
    {
        NodeFrameTasksFinishPacket()
            {
                command     = fabric::CMD_NODE_FRAME_TASKS_FINISH;
                size        = sizeof( NodeFrameTasksFinishPacket );
            }

        uint128_t frameID;
        uint32_t frameNumber;
    };

    struct NodeAffinityPacket : public NodePacket
    {
        NodeAffinityPacket()
              {
                  command = fabric::CMD_NODE_SET_AFFINITY;
                  size = sizeof(NodeAffinityPacket);
              }

        int32_t affinity;
    };

    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeCreatePipePacket* packet )
    {
        os << (co::ObjectPacket*)packet << " id " << packet->pipeID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeFrameStartPacket* packet )
    {
        os << (co::ObjectPacket*)packet << " frame " << packet->frameNumber
           << " id " << packet->frameID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeFrameDrawFinishPacket* packet )
    {
        os << (co::ObjectPacket*)packet << " frame " << packet->frameNumber
           << " id " << packet->frameID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeFrameFinishPacket* packet )
    {
        os << (co::ObjectPacket*)packet << " frame " << packet->frameNumber
           << " id " << packet->frameID;
        return os;
    }
}
/** @endcond */
#endif //EQ_NODEPACKETS_H
