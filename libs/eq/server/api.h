
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_API_H
#define EQSERVER_API_H

#include <eq/client/defines.h>
#include <eq/fabric/api.h>

#if defined(EQUALIZERSERVER_STATIC)
#  define EQSERVER_API
#elif defined(EQUALIZERSERVER_SHARED)
#  define EQSERVER_API LB_DLLEXPORT
#else
#  define EQSERVER_API LB_DLLIMPORT
#endif

#endif // EQSERVER_API_H
