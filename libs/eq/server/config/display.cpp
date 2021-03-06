
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.h> 
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

#include "display.h"

#include "resources.h"
#include "../canvas.h"
#include "../channel.h"
#include "../config.h"
#include "../layout.h"
#include "../node.h"
#include "../pipe.h"
#include "../observer.h"
#include "../segment.h"
#include "../view.h"
#include "../window.h"

#include <eq/client/configParams.h>

namespace eq
{
namespace server
{
namespace config
{

void Display::discoverLocal( Config* config, const uint32_t flags )
{
    Node* node = config->findAppNode();
    LBASSERT( node );
    if( !node )
        return;

    const Pipes& pipes = node->getPipes();
    LBASSERT( !pipes.empty( ));
    if( pipes.empty( ))
        return;

    Pipe* pipe = pipes.front();
    Window* window = new Window( pipe );
    window->setViewport( Viewport( .25f, .2f, .5f, .5f ));
    window->setName( pipe->getName() + " window" );
    window->setIAttribute( Window::IATTR_PLANES_STENCIL, 1 );

    Channel* channel = new Channel( window );
    channel->setName( pipe->getName() + " channel" );
    Observer* observer = new Observer( config );

    const PixelViewport& pvp = pipe->getPixelViewport();
    Wall wall;
    if( pvp.isValid( ))
        wall.resizeHorizontalToAR( float( pvp.w ) / float( pvp.h ));

    Canvas* canvas = new Canvas( config );
    canvas->setWall( wall );

    Segment* segment = new Segment( canvas );
    segment->setChannel( channel );

    Strings names;
    const Nodes& nodes = config->getNodes();
    const bool scalability = nodes.size() > 1 || pipes.size() > 1;

    if( scalability )
        names.push_back( EQ_SERVER_CONFIG_LAYOUT_2D_DYNAMIC );

    names.push_back( EQ_SERVER_CONFIG_LAYOUT_SIMPLE );

    if( scalability )
    {
        names.push_back( EQ_SERVER_CONFIG_LAYOUT_DB_DS );
        names.push_back( EQ_SERVER_CONFIG_LAYOUT_DB_STATIC );
        names.push_back( EQ_SERVER_CONFIG_LAYOUT_DB_DYNAMIC );
        if( flags & ConfigParams::FLAG_MULTIPROCESS_DB && nodes.size() > 1 )
        {
            for( NodesCIter i = nodes.begin(); i != nodes.end(); ++i )
            {
                if( (*i)->getPipes().size() > 1 )
                {
                    names.push_back( EQ_SERVER_CONFIG_LAYOUT_DB_2D );
                    break;
                }
            }
        }
    }

    for( StringsCIter i = names.begin(); i != names.end(); ++i )
    {
        Layout* layout = new Layout( config );
        layout->setName( *i );

        View* view = new View( layout );
        view->setObserver( observer );
        view->setWall( wall );

        canvas->addLayout( layout );
    }

    config->activateCanvas( canvas );
}

}
}
}
