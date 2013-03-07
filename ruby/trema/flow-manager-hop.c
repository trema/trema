/*
 * Copyright (C) 2008-2013 NEC Corporation
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

#include <string.h>
#include "trema.h"
#include "ruby.h"
#include "path.h"
#include "flow-manager-hop.h"
#include "path_utils.h"
#include "action-common.h"

//Please uncomment below when you need debug output
//#define DEBUG
#ifdef DEBUG
#define debug(...) {printf("%s(%d):", __func__, __LINE__); printf(__VA_ARGS__);}
#else
#define debug(...) 1 ? (void) 0 : printf
#endif

typedef struct {
  hop public;
  void *r_hop_pointer;
  void *r_extra_actions_pointer;
} hop_private;

extern VALUE mTrema;
VALUE cHop;

static void form_actions( VALUE raction, openflow_actions *actions );
static void flow_manager_append_action( openflow_actions *actions, VALUE action );

static void form_actions( VALUE raction, openflow_actions *actions )
{
  debug("start\n");
  if ( raction != Qnil ) {
    switch ( TYPE( raction ) ) {
      case T_ARRAY:
        {
          debug("actions Type : T_ARRAY\n");
          VALUE *each = RARRAY_PTR( raction );
          int i;
          for ( i = 0; i < RARRAY_LEN( raction ); i++ ) {
        	  flow_manager_append_action( actions, each[ i ] );
          }
          debug("check actions array (number) : %d\n", actions->n_actions);
        }
        break;
      case T_OBJECT:
        debug("actions Type : T_OBJECT\n");
        flow_manager_append_action( actions, raction );
        break;
      default:
        debug("actions Type : default\n");
        rb_raise( rb_eTypeError, "actions argument must be an Array or an Action object" );
        break;
    }
  }
  debug("end\n");
}

//Expect the arguments are checked on Ruby Action Class side
static void flow_manager_append_action( openflow_actions *actions, VALUE action ) {
  if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::Enqueue" ) ) == Qtrue ) {
    uint32_t queue_id = ( uint32_t ) NUM2UINT( rb_funcall( action, rb_intern( "queue_id" ), 0 ) );
    uint16_t port_number = ( uint16_t ) NUM2UINT( rb_funcall( action, rb_intern( "port_number" ), 0 ) );
    append_action_enqueue( actions, port_number, queue_id );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::SendOutPort" ) ) == Qtrue ) {
    uint16_t port_number = ( uint16_t ) NUM2UINT( rb_funcall( action, rb_intern( "port_number" ), 0 ) );
    uint16_t max_len = ( uint16_t ) NUM2UINT( rb_funcall( action, rb_intern( "max_len" ), 0 ) );
    append_action_output( actions, port_number, max_len );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::SetEthDstAddr" ) ) == Qtrue ) {
    uint8_t dl_dst[ OFP_ETH_ALEN ];
    uint8_t *ptr = ( uint8_t* ) dl_addr_to_a( rb_funcall( action, rb_intern( "mac_address" ), 0 ), dl_dst );
    append_action_set_dl_dst( actions, ptr );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::SetEthSrcAddr" ) ) == Qtrue ) {
    uint8_t dl_src[ OFP_ETH_ALEN ];
    uint8_t *ptr = ( uint8_t* ) dl_addr_to_a( rb_funcall( action, rb_intern( "mac_address" ), 0 ), dl_src );
    append_action_set_dl_src( actions, ptr );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::SetIpDstAddr" ) ) == Qtrue ) {
    append_action_set_nw_dst( actions, nw_addr_to_i( rb_funcall( action, rb_intern( "ip_address" ), 0 ) ) );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::SetIpSrcAddr" ) ) == Qtrue ) {
    append_action_set_nw_src( actions, nw_addr_to_i( rb_funcall( action, rb_intern( "ip_address" ), 0 ) ) );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::SetIpTos" ) ) == Qtrue ) {
    append_action_set_nw_tos( actions, ( uint8_t ) NUM2UINT( rb_funcall( action, rb_intern( "type_of_service" ), 0 ) ) );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::SetTransportDstPort" ) ) == Qtrue ) {
    append_action_set_tp_dst( actions, ( uint16_t ) NUM2UINT( rb_funcall( action, rb_intern( "port_number" ), 0 ) ) );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::SetTransportSrcPort" ) ) == Qtrue ) {
    append_action_set_tp_src( actions, ( uint16_t ) NUM2UINT( rb_funcall( action, rb_intern( "port_number" ), 0 ) ) );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::SetVlanPriority" ) ) == Qtrue ) {
    append_action_set_vlan_pcp( actions, ( uint8_t ) NUM2UINT( rb_funcall( action, rb_intern( "vlan_priority" ), 0 ) ) );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::SetVlanVid" ) ) == Qtrue ) {
    append_action_set_vlan_vid( actions, ( uint16_t ) NUM2UINT( rb_funcall( action, rb_intern( "vlan_id" ), 0 ) ) );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::StripVlanHeader" ) ) == Qtrue ) {
    append_action_strip_vlan( actions );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::VendorAction" ) ) == Qtrue ) {
    VALUE vendor_id = rb_funcall( action, rb_intern( "vendor_id" ), 0 );
    VALUE rbody = rb_funcall( action, rb_intern( "body" ), 0 );
    if ( rbody != Qnil ) {
      Check_Type( rbody, T_ARRAY );
      uint16_t length = ( uint16_t ) RARRAY_LEN( rbody );
      buffer *body = alloc_buffer_with_length( length );
      int i;
      for ( i = 0; i < length; i++ ) {
        ( ( uint8_t * ) body->data )[ i ] = ( uint8_t ) FIX2INT( RARRAY_PTR( rbody )[ i ] );
      }
      append_action_vendor( actions, ( uint32_t ) NUM2UINT( vendor_id ), body );
      free_buffer( body );
    }
    else {
      append_action_vendor( actions, ( uint32_t ) NUM2UINT( vendor_id ), NULL );
    }
  }
  else {
    rb_raise( rb_eTypeError, "actions argument must be an Array of Action objects" );
  }
}

/*
 * The value of datapath id
 *
 * @return [Number]
 */
static VALUE hop_datapath_id(VALUE self)
{
	hop *h;
	Data_Get_Struct(self, hop, h);
	return UINT2NUM((long unsigned int)h->datapath_id);
}

/*
 * The value of in port
 *
 * @return [Number]
 */
static VALUE hop_in_port(VALUE self)
{
	hop *h;
	Data_Get_Struct(self, hop, h);
	return UINT2NUM(h->in_port);
}

/*
 * The value of out port
 *
 * @return [Number]
 */
static VALUE hop_out_port(VALUE self)
{
	hop *h;
	Data_Get_Struct(self, hop, h);
	return UINT2NUM(h->out_port);
}

/*
 * The value of actions
 *
 * @return [Array]
 */
static VALUE hop_actions(VALUE self)
{
	hop_private *hp;
	Data_Get_Struct(self, hop_private, hp);
	return (VALUE)hp->r_extra_actions_pointer;
}

/*
 * Creates a {Hop} instance.
 *
 * @overload initialize(datapath_id, in_port, out_port, actions)
 *
 *   @example
 *     Hop.new(
 *       Number datapath_id,
 *       Number in_port,
 *       Number out_port,
 *       Array actions,
 *     )
 *
 *	@param [Number] datapath_id datapath id.
 *
 *	@param [Number] in_port in port.
 *
 *	@param [Number] out_port out port.
 *
 *	@param [Array] actions.
 *
 *	@return [Hop] self an object that encapsulates and wraps the +hop+
 */
static VALUE hop_initialize(int argc, VALUE *argv, VALUE self)
{
        hop_private *hp;
        Data_Get_Struct(self, hop_private, hp);

        VALUE datapath_id;
        VALUE in_port;
        VALUE out_port;
        VALUE actions;
        int nargs = rb_scan_args(argc, argv, "31", &datapath_id, &in_port, &out_port, &actions);

        if(NUM2LONG(datapath_id)<0 || NUM2INT(in_port)<0 || NUM2INT(out_port)<0)
        {
          rb_raise( rb_eRangeError, "Please input positive integer." );
        }

        openflow_actions *_actions;

        switch(nargs)
        {
                case 3:
                {
                  hp->public.datapath_id = (uint64_t)NUM2UINT(datapath_id);
                  hp->public.in_port = (uint16_t)NUM2INT(in_port);
                  hp->public.out_port = (uint16_t)NUM2INT(out_port);
                  hp->public.extra_actions = NULL;

                  break;
                }
                case 4:
                {
                  hp->public.datapath_id = (uint64_t)NUM2UINT(datapath_id);
                  hp->public.in_port = (uint16_t)NUM2INT(in_port);
                  hp->public.out_port = (uint16_t)NUM2INT(out_port);
                  hp->public.extra_actions = NULL;

                  _actions = create_actions();

                  debug("actions %p is created", _actions);

                  form_actions(actions, _actions);
                  if(_actions != NULL){
                          hp->public.extra_actions = _actions;
                  }

                  hp->r_extra_actions_pointer = (void *)actions;

                  break;
                }
                default:
                {
                    //This pass never be passed because of pre-argument-check.
                    rb_raise( rb_eArgError, "The number of argument is invalid." );
                    break;
                }
        }

        return Qnil;
}

static VALUE create_Hop(VALUE klass)
{
	hop_private *ph = ALLOC( hop_private );
	memset( ph, 0, sizeof( hop_private ) );
	debug("hop_private %p is created\n", ph);
	hop *h = &ph->public;
	debug("dpid : %d", h->datapath_id);
	debug("hop pointer is %p\n", h);
	VALUE rHop = Data_Wrap_Struct(klass, 0, -1, h );
	debug("ruby hop %p is created\n", ph->r_hop_pointer);
	ph->r_hop_pointer = (void *)rHop;
	return rHop;
}

/*
 * Document-class: Trema::Hop
 */
void Init_hop()
{
	cHop = rb_define_class_under(mTrema, "Hop", rb_cObject);
	rb_define_alloc_func(cHop, create_Hop);
	rb_define_method(cHop, "initialize", hop_initialize, -1);
	rb_define_method(cHop, "datapath_id", hop_datapath_id, 0);
	rb_define_method(cHop, "in_port", hop_in_port, 0);
	rb_define_method(cHop, "out_port", hop_out_port, 0);
	rb_define_method(cHop, "actions", hop_actions, 0);
}
