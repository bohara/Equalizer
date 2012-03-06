
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

#ifndef EQ_CHANNELPACKETS_H
#define EQ_CHANNELPACKETS_H

#include <eq/client/packets.h> // base structs
#include <eq/client/statistic.h> // member
#include <eq/fabric/renderContext.h> // member

/** @cond IGNORE */
namespace eq
{
    struct ChannelConfigInitPacket : public ChannelPacket
    {
        ChannelConfigInitPacket( const uint128_t& initID_ )
                : initID( initID )
            {
                command = fabric::CMD_CHANNEL_CONFIG_INIT;
                size    = sizeof( ChannelConfigInitPacket );
            }

        const uint128_t initID;
    };

    struct ChannelConfigInitReplyPacket : public ChannelPacket
    {
        ChannelConfigInitReplyPacket()
            {
                command   = fabric::CMD_CHANNEL_CONFIG_INIT_REPLY;
                size      = sizeof( ChannelConfigInitReplyPacket );
            }

        bool result;
    };

    struct ChannelConfigExitPacket : public ChannelPacket
    {
        ChannelConfigExitPacket()
            {
                command = fabric::CMD_CHANNEL_CONFIG_EXIT;
                size    = sizeof( ChannelConfigExitPacket );
            }
    };

    struct ChannelConfigExitReplyPacket : public ChannelPacket
    {
        ChannelConfigExitReplyPacket( const co::base::UUID& channelID, const bool res )
                : result( res )
            {
                command   = fabric::CMD_CHANNEL_CONFIG_EXIT_REPLY;
                size      = sizeof( ChannelConfigExitReplyPacket );
                objectID  = channelID;
            }

        const bool result;
    };

    struct ChannelTaskPacket : public ChannelPacket
    {
        RenderContext context;
    };

    struct ChannelFrameStartPacket : public ChannelTaskPacket
    {
        ChannelFrameStartPacket()
            {
                command        = fabric::CMD_CHANNEL_FRAME_START;
                size           = sizeof( ChannelFrameStartPacket );
            }

        uint128_t version;
        uint32_t  frameNumber;
    };  

    struct ChannelStopFramePacket : public co::ObjectPacket
    {
        ChannelStopFramePacket()
            {
                command       = fabric::CMD_CHANNEL_STOP_FRAME;
                size          = sizeof( ChannelStopFramePacket );
            }
        uint32_t lastFrameNumber;
    };

    struct ChannelFrameFinishPacket : public ChannelTaskPacket
    {
        ChannelFrameFinishPacket()
            {
                command        = fabric::CMD_CHANNEL_FRAME_FINISH;
                size           = sizeof( ChannelFrameFinishPacket );
            }

        uint32_t frameNumber;
    };

    struct ChannelFrameFinishReplyPacket : public ChannelPacket
    {
        ChannelFrameFinishReplyPacket( )
            {
                command     = fabric::CMD_CHANNEL_FRAME_FINISH_REPLY;
                size        = sizeof( ChannelFrameFinishReplyPacket );
            }

        Viewport region;
        uint32_t frameNumber;
        uint32_t nStatistics;
        EQ_ALIGN8( Statistic statistics[1] );
    };
        

    struct ChannelFrameDrawFinishPacket : public ChannelPacket
    {
        ChannelFrameDrawFinishPacket()
            {
                command       = fabric::CMD_CHANNEL_FRAME_DRAW_FINISH;
                size          = sizeof( ChannelFrameDrawFinishPacket );
            }

        uint128_t frameID;
        uint32_t  frameNumber;
    };
        
    struct ChannelFrameClearPacket : public ChannelTaskPacket
    {
        ChannelFrameClearPacket()
            {
                command       = fabric::CMD_CHANNEL_FRAME_CLEAR;
                size          = sizeof( ChannelFrameClearPacket );
            }
    };
        
    struct ChannelFrameDrawPacket : public ChannelTaskPacket
    {
        ChannelFrameDrawPacket()
                : finish( false )
            {
                command       = fabric::CMD_CHANNEL_FRAME_DRAW;
                size          = sizeof( ChannelFrameDrawPacket );
            }
        bool finish;
    };
        
    struct ChannelFrameAssemblePacket : public ChannelTaskPacket
    {
        ChannelFrameAssemblePacket()
            {
                command       = fabric::CMD_CHANNEL_FRAME_ASSEMBLE;
                size          = sizeof( ChannelFrameAssemblePacket );
            }

        uint32_t             nFrames;
        EQ_ALIGN8( co::ObjectVersion frames[1] );
    };
        
    struct ChannelFrameReadbackPacket : public ChannelTaskPacket
    {
        ChannelFrameReadbackPacket()
            {
                command       = fabric::CMD_CHANNEL_FRAME_READBACK;
                size          = sizeof( ChannelFrameReadbackPacket );
            }

        uint32_t             nFrames;
        EQ_ALIGN8( co::ObjectVersion frames[1] );
    };

    struct ChannelFrameTransmitImagePacket : public ChannelTaskPacket
    {
        ChannelFrameTransmitImagePacket()
                : fill( 0 )
        {
            command       = fabric::CMD_CHANNEL_FRAME_TRANSMIT_IMAGE;
            size          = sizeof( ChannelFrameTransmitImagePacket );
        }

        co::ObjectVersion  frameData;
        uint128_t          netNodeID;
        uint128_t          clientNodeID;
        uint64_t           imageIndex;
        uint32_t           frameNumber;
        const uint32_t     fill;
    };

    struct ChannelFrameSetReadyPacket : public ChannelFrameTransmitImagePacket
    {
        ChannelFrameSetReadyPacket()
        {
            command       = fabric::CMD_CHANNEL_FRAME_SET_READY;
            size          = sizeof( ChannelFrameSetReadyPacket );
        }
    };

    struct ChannelFrameViewStartPacket : public ChannelTaskPacket
    {
        ChannelFrameViewStartPacket()
            {
                command       = fabric::CMD_CHANNEL_FRAME_VIEW_START;
                size          = sizeof( ChannelFrameViewStartPacket );
            }
    };

    struct ChannelFrameViewFinishPacket : public ChannelTaskPacket
    {
        ChannelFrameViewFinishPacket()
            {
                command       = fabric::CMD_CHANNEL_FRAME_VIEW_FINISH;
                size          = sizeof( ChannelFrameViewFinishPacket );
            }
    };

    struct ChannelFrameTilesPacket : public ChannelTaskPacket
    {
        ChannelFrameTilesPacket()
        {
            command           = fabric::CMD_CHANNEL_FRAME_TILES;
            size              = sizeof( ChannelFrameTilesPacket );
        }

        bool              isLocal;
        co::ObjectVersion queueVersion;
        uint32_t          tasks;
        uint32_t          nFrames;
        EQ_ALIGN8( co::ObjectVersion frames[1] );
    };

    inline std::ostream& operator << ( std::ostream& os, 
                                    const ChannelConfigInitReplyPacket* packet )
    {
        os << (co::ObjectPacket*)packet << " result " << packet->result;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const ChannelFrameStartPacket* packet )
    {
        os << (co::ObjectPacket*)packet << " frame " << packet->frameNumber;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                   const ChannelFrameFinishReplyPacket* packet )
    {
        os << (co::ObjectPacket*)packet << " frame " << packet->frameNumber;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                    const ChannelFrameDrawFinishPacket* packet )
    {
        os << (co::ObjectPacket*)packet << " frame " << packet->frameNumber
           << " id " << packet->frameID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const ChannelTaskPacket* packet )
    {
        os << (co::ObjectPacket*)packet << " " << packet->context;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                      const ChannelFrameReadbackPacket* packet )
    {
        os << (ChannelTaskPacket*)packet << " nFrames " << packet->nFrames;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                     const ChannelFrameTransmitImagePacket* packet )
    {
        os << (co::ObjectPacket*)packet << " frame data " << packet->frameData
           << " receiver " << packet->clientNodeID << " on "
           << packet->netNodeID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                      const ChannelFrameAssemblePacket* packet )
    {
        os << (ChannelTaskPacket*)packet << " nFrames " << packet->nFrames;
        return os;
    }
}
/** @endcond */
#endif //EQ_CHANNELPACKETS_H
