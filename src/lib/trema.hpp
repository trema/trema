/*
 * Trema C++ wrapper.
 *
 * Copyright (C) 2012 Vladimir Olteanu
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifndef TREMA_HPP
#define TREMA_HPP


template <typename type1, typename type2> struct _types_compatible_p {
  static const bool result = false;
};

template <typename type1> struct _types_compatible_p<type1, type1> {
  static const bool result = true;
};

#define __builtin_types_compatible_p( type1, type2 ) _types_compatible_p< type1, type2 >::result


extern "C" {
#include "trema.h"
}


#endif // TREMA_HPP


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
