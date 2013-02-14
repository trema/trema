/*
 * Copyright (C) 2011 NEC Corporation
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


#include "path.h"
#include "trema.h"
#include "openflow_message.h"
#include "path_utils.h"
#include "utility.h"

static void dump_action( struct ofp_action_header *action )
{
  if (( action->len ) < sizeof( struct ofp_action_header ) ) {
      info("ERROR_TOO_SHORT_ACTION");
  }

  char buf[256];
  actions_to_string(action, action->len, buf, sizeof(buf) );
  info("%s", buf);
}

void
dump_match( const struct ofp_match *match ) {
  char match_string[ 256 ];
  match_to_string( match, match_string, sizeof( match_string ) );
  info( "Match: match = [%s]",match_string);
}

void
dump_hop( const hop *h ) {

  if(h->extra_actions == NULL)
  {
    info( "Hop: datapath_id = %#" PRIx64 ", in_port = %u, out_port = %u, n_actions = 0.",
          h->datapath_id, h->in_port, h->out_port);
  }
  else
  {
    info( "Hop: datapath_id = %#" PRIx64 ", in_port = %u, out_port = %u, n_actions = %u.",
          h->datapath_id, h->in_port, h->out_port, h->extra_actions->n_actions);

    list_element *e = h->extra_actions->list;
    while( e != NULL)
    {
        dump_action( e->data );
        e = e->next;
    }
  }
}

void
dump_path( const path *p ) {
  char match_string[ 256 ];
  match_to_string( &p->match, match_string, sizeof( match_string ) );

  /*
  info( "Path: match = [%s], priority = %u, idle_timeout = %u, hard_timeout = %u, n_hops = %d, hops = %p.",
        match_string, p->priority, p->idle_timeout, p->hard_timeout, p->n_hops, p->hops );
  */

  info( "Path: match = [%s], priority = %u, idle_timeout = %u, hard_timeout = %u, n_hops = %d.",
        match_string, p->priority, p->idle_timeout, p->hard_timeout, p->n_hops);
  if ( p->n_hops > 0 && p->hops != NULL ) {
    list_element *e = p->hops;
    while ( e != NULL ) {
      dump_hop( e->data );
      e = e->next;
    }
  }
}

const char *
status_to_string( int status ) {
  switch ( status ) {
    case SETUP_SUCCEEDED:
      return "succeeded";
    break;
    case SETUP_CONFLICTED_ENTRY:
      return "conflicted entry";
    break;
    case SETUP_SWITCH_ERROR:
      return "switch error";
    break;
    default:
    break;
  }

  return "undefined";
}


const char *
reason_to_string( int reason ) {
  switch ( reason ) {
    case TEARDOWN_TIMEOUT:
      return "timeout";
    break;
    case TEARDOWN_MANUALLY_REQUESTED:
      return "manually requested";
    break;
    default:
    break;
  }

  return "undefined";
}
