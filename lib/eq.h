
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_H
#define EQ_H

#include <eq/base/defines.h>

#include <eq/client/channelStatistics.h>
#include <eq/client/client.h>
#include <eq/client/compositor.h>
#include <eq/client/config.h>
#include <eq/client/configEvent.h>
#include <eq/client/configParams.h>
#include <eq/client/event.h>
#include <eq/client/frame.h>
#include <eq/client/frameData.h>
#include <eq/client/global.h>
#include <eq/client/image.h>
#include <eq/client/init.h>
#include <eq/client/log.h>
#include <eq/client/matrix4.h>
#include <eq/client/node.h>
#include <eq/client/nodeFactory.h>
#include <eq/client/objectManager.h>
#include <eq/client/packets.h>
#include <eq/client/pipe.h>
#include <eq/client/server.h>
#include <eq/client/version.h>
#include <eq/client/view.h>
#include <eq/client/windowSystem.h>

#ifdef AGL
#  include <eq/client/aglWindow.h>
#  include <eq/client/aglEventHandler.h>
#endif
#ifdef GLX
#  include <eq/client/glXWindow.h>
#  include <eq/client/glXEventHandler.h>
#endif
#ifdef WGL
#  include <eq/client/wglWindow.h>
#  include <eq/client/wglEventHandler.h>
#endif

#include <eq/base/sleep.h>
#include <eq/net/net.h>
#include <eq/util/util.h>

#include <vmmlib/vmmlib.h>

#ifdef EQ_USE_DEPRECATED
namespace eqBase = ::eq::base;
namespace eqNet  = ::eq::net;
#endif

#endif // EQ_H
