
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch>
 *               2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQSEQUEL_DETAIL_VIEW_H
#define EQSEQUEL_DETAIL_VIEW_H

#include <seq/types.h>
#include <eq/client/view.h> // base class

namespace seq
{
namespace detail
{
    class View : public eq::View
    {
    public:
        View( eq::Layout* parent );

        /** @name Data Access. */
        //@{
        Config* getConfig();
        Pipe* getPipe();
        ViewData* getViewData();
        const ViewData* getViewData() const;
        //@}

        /** @name Operations. */
        //@{
#ifndef EQ_2_0_API
        bool handleEvent( const eq::ConfigEvent* event );
#endif
        bool handleEvent( const eq::EventCommand& command );
        bool updateData();
        //@}

    protected:
        virtual ~View();
        virtual void notifyAttach();
        virtual void notifyDetached();

    private:
    };
}
}

#endif // EQSEQUEL_DETAIL_VIEW_H
