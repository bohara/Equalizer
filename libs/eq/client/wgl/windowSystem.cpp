
/* Copyright (c) 2011, Daniel Pfeifer <daniel@pfeifer-mail.de>
 *               2011, Stefan Eilemann <eile@eyescale.ch>
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

#include "../windowSystem.h"

#include "eventHandler.h"
#include "messagePump.h"
#include "pipe.h"
#include "window.h"

#include "../config.h"
#include "../node.h"
#include "../pipe.h"
#include "../server.h"

#include <eq/fabric/gpuInfo.h>

namespace eq
{
namespace wgl
{

static class : WindowSystemIF
{
    std::string getName() const { return "WGL"; }

    eq::SystemWindow* createWindow(eq::Window* window) const
    {
        LBINFO << "Using wgl::Window" << std::endl;
        return new Window(window);
    }

    eq::SystemPipe* createPipe(eq::Pipe* pipe) const
    {
        LBINFO << "Using wgl::Pipe" << std::endl;
        return new Pipe(pipe);
    }

    eq::MessagePump* createMessagePump() const
    {
        return new MessagePump;
    }

    bool setupFont( ObjectManager& gl, const void* key, const std::string& name,
                    const uint32_t size ) const
    {
        HDC dc = wglGetCurrentDC();
        if( !dc )
        {
            LBWARN << "No WGL device context current" << std::endl;
            return false;
        }

        LOGFONT font;
        memset( &font, 0, sizeof( font ));
        font.lfHeight = -static_cast< LONG >( size );
        font.lfWeight = FW_NORMAL;
        font.lfCharSet = ANSI_CHARSET;
        font.lfOutPrecision = OUT_DEFAULT_PRECIS;
        font.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        font.lfQuality = DEFAULT_QUALITY;
        font.lfPitchAndFamily = FF_DONTCARE | DEFAULT_QUALITY;

        if( name.empty( ))
            strncpy( font.lfFaceName, "Times New Roman", LF_FACESIZE );
        else
            strncpy( font.lfFaceName, name.c_str(), LF_FACESIZE );

        font.lfFaceName[ LF_FACESIZE-1 ] = '\0';

        HFONT newFont = CreateFontIndirect( &font );
        if( !newFont )
        {
            LBWARN << "Can't load font " << name << ", using Times New Roman" 
                   << std::endl;

            strncpy( font.lfFaceName, "Times New Roman", LF_FACESIZE );
            newFont = CreateFontIndirect( &font );
        }
        LBASSERT( newFont );

        HFONT oldFont = static_cast< HFONT >( SelectObject( dc, newFont ));

        const GLuint lists = _setupLists( gl, key, 256 );
        const bool ret = wglUseFontBitmaps( dc, 0 , 255, lists );
    
        SelectObject( dc, oldFont );
        //DeleteObject( newFont );
    
        if( !ret )
            _setupLists( gl, key, 0 );

        return ret;
    }

    void configInit( eq::Node* node )
    {
#ifdef EQ_USE_MAGELLAN
        EventHandler::initMagellan( node );
#endif
    }

    void configExit( eq::Node* node )
    {
#ifdef EQ_USE_MAGELLAN
        EventHandler::exitMagellan( node );
#endif
    }
} _wglFactory;

}
}
