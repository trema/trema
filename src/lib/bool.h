/*
 * bool type definition.
 *
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
 *
 * Copyright (C) 2008-2011 NEC Corporation
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


/**
 * @file
 *
 * @brief Utility file containing enumerator for Boolean type 
 *
 * Utility file containing enumerator for Boolean type, defines FALSE as 0 and 1
 * as TRUE. This is being widely used within the TREMA code for function
 * returning boolean types.
 */

#ifndef BOOL_H
#define BOOL_H


/**
 * In case C++ compiler is being used, not bothering to define a new
 * enumerator; C++'s inbuilt boolean type is used
 */
#ifdef __cplusplus
// use C++ built-in bool type
#elif defined __STDC__ && defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
// have a C99 compiler
#include <stdbool.h>
#else
// do not have a C99 compiler
typedef enum bool {
  false = 0,
  true,
} bool;
#endif


#endif // BOOL_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
