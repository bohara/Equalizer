
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Equalizer 'Hello, World!' example. Shows the minimum Equalizer program,
 * rendering spinning quads around the origin.
 */

#include <seq/sequel.h>
#include <stdlib.h>

namespace eqHello
{
class Renderer : public seq::Renderer
{
public:
    Renderer( seq::Application& application ) : seq::Renderer( application ) {}
    virtual ~Renderer() {}

protected:
    virtual void draw( co::Object* frameData );
};

class Application : public seq::Application
{
public:
    virtual ~Application() {}
    virtual seq::Renderer* createRenderer() { return new Renderer( *this ); }
};
typedef lunchbox::RefPtr< Application > ApplicationPtr;
}

int main( const int argc, char** argv )
{
    eqHello::ApplicationPtr app = new eqHello::Application;

    if( app->init( argc, argv, 0 ) && app->run( 0 ) && app->exit( ))
        return EXIT_SUCCESS;
    
    return EXIT_FAILURE;
}

/** The rendering routine, a.k.a., glutDisplayFunc() */
void eqHello::Renderer::draw( co::Object* frameData )
{
    applyRenderContext(); // set up OpenGL State

    const float lightPos[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    glLightfv( GL_LIGHT0, GL_POSITION, lightPos );

    const float lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightfv( GL_LIGHT0, GL_AMBIENT, lightAmbient );

    applyModelMatrix(); // global camera

    // render six axis-aligned colored quads around the origin
    for( int i = 0; i < 6; i++ )
    {
        glColor3f( (i&1) ? 0.5f:1.0f, (i&2) ? 1.0f:0.5f, (i&4) ? 1.0f:0.5f );

        glNormal3f( 0.0f, 0.0f, 1.0f );
        glBegin( GL_TRIANGLE_STRIP );
            glVertex3f(  .7f,  .7f, -1.0f );
            glVertex3f( -.7f,  .7f, -1.0f );
            glVertex3f(  .7f, -.7f, -1.0f );
            glVertex3f( -.7f, -.7f, -1.0f );
        glEnd();

        if( i < 3 )
            glRotatef(  90.0f, 0.0f, 1.0f, 0.0f );
        else if( i == 3 )
            glRotatef(  90.0f, 1.0f, 0.0f, 0.0f );
        else
            glRotatef( 180.0f, 1.0f, 0.0f, 0.0f );
    }
}
