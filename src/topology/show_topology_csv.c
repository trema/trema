/*
 * Author: Shuji Ishii, Kazushi SUGYO
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


#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>
#include "trema.h"
#include "libtopology.h"
#include "show_topology.h"
#include "topology_service_interface_option_parser.h"


static void
print_link_status( const topology_link_status *s ) {
  const char *status = "unknown";
  switch ( s->status ) {
    case TD_LINK_DOWN:
      status = "down";
      break;
    case TD_LINK_UP:
      status = "up";
      break;
    case TD_LINK_UNSTABLE:
      status = "unstable";
      break;
  }
  if ( s->status == TD_LINK_UP ) {
    printf( "0x%" PRIx64 ",%u,0x%" PRIx64 ",%u,%s\n",
            s->from_dpid, s->from_portno, s->to_dpid, s->to_portno, status );
  }
  else {
    printf( "0x%" PRIx64 ",%u,-,-,%s\n",
            s->from_dpid, s->from_portno, status );
  }
}


static void
insert_data( list_element **head, const topology_link_status *s ) {
  list_element *element;
  for ( element = *head; element != NULL; element = element->next ) {
    const topology_link_status *entry = element->data;
    if ( entry->from_dpid > s->from_dpid ) {
      break;
    }
    if ( entry->from_dpid < s->from_dpid ) {
      continue;
    }
    if ( entry->from_portno > s->from_portno ) {
      break;
    }
    if ( entry->from_portno < s->from_portno ) {
      continue;
    }
    if ( entry->to_dpid > s->to_dpid ) {
      break;
    }
    if ( entry->to_dpid < s->to_dpid ) {
      continue;
    }
    if ( entry->to_portno > s->to_portno ) {
      break;
    }
  }
  if ( element == NULL ) {
    append_to_tail( head, ( void * ) ( intptr_t ) s );
  }
  else if ( element == *head ) {
    insert_in_front( head, ( void * ) ( intptr_t ) s );
  }
  else {
    insert_before( head, element->data, ( void * ) ( intptr_t ) s );
  }
}


void
print_with_csv_format( void *param, size_t entries, const topology_link_status *s ) {
  size_t i;

  UNUSED( param );

  debug( "topology: entries %zd", entries );

  list_element *link;
  create_list( &link );

  for ( i = 0; i < entries; i++ ) {
    insert_data( &link, &s[ i ] );
  }

  printf( "f-dpid,f-port,t-dpid,t-port,stat\n" );
  list_element *element;
  for ( element = link; element != NULL; element = element->next ) {
    print_link_status( element->data );
  }

  delete_list( link );

  stop_trema();
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
