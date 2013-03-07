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
 *
 */

#include <string.h>
#include <memory.h>
#include "trema.h"
#include "openflow.h"
#include "ruby.h"
#include "path.h"
#include "match.h"
#include "flow-manager-hop.h"
#include "flow-manager-module.h"
#include "path_utils.h"

//Please uncomment below when you need debug output
//#define DEBUG
#ifdef DEBUG
#define debug(...) {printf("%s(%d):", __func__, __LINE__); printf(__VA_ARGS__);}
#else
#define debug(...) 1 ? (void) 0 : printf
#endif

//This structure exists in path.c
typedef struct {
  path public;
  uint64_t id;
  uint64_t in_datapath_id;
  setup_handler setup_callback;
  void *setup_user_data;
  teardown_handler teardown_callback;
  void *teardown_user_data;
} path_private;

typedef struct {
  hop public;
  void *r_hop_pointer;
  void *r_extra_actions_pointer;
} hop_private;

extern VALUE mTrema;
VALUE cPath;

/*
 * The value of actions
 *
 * @return [Array]
 */
static VALUE path_priority(VALUE self)
{
    path *p;
    Data_Get_Struct(self, path, p);
    return UINT2NUM(p->priority);
}

/*
 * The value of idle timeout
 *
 * @return [Number]
 */
static VALUE path_idle_timeout(VALUE self)
{
    path *p;
    Data_Get_Struct(self, path, p);
    return UINT2NUM(p->idle_timeout);
}

/*
 * The value of hard timeout
 *
 * @return [Number]
 */
static VALUE path_hard_timeout(VALUE self)
{
    path *p;
    Data_Get_Struct(self, path, p);
    return UINT2NUM(p->hard_timeout);
}

/*
 * The value of hops
 *
 * @return [Array]
 */
static VALUE path_hops(VALUE self)
{
    debug("start\n");

    VALUE hops = rb_ary_new();

    path *p;
    Data_Get_Struct(self, path, p);

    list_element *element = p->hops;
    while ( element != NULL ) {
            hop_private *temp = (hop_private *)element->data;

            //VALUE rHop = Data_Wrap_Struct(cHop, 0, -1, &temp->public );
            //rb_ary_push(hops, rHop);
            rb_ary_push(hops, (VALUE)temp->r_hop_pointer);

            element = element->next;
    }

    debug("end\n");
    return hops;
}

/*
 * The value of match
 *
 * @return [Match]
 */
static VALUE path_match(VALUE self)
{
    debug("start\n");
    path *p;
    Data_Get_Struct(self, path, p);
    debug(dump_match(&p->match));

    VALUE obj;
    obj = rb_funcall( cMatch, rb_intern( "new" ), 0 );
    struct ofp_match *match;
    Data_Get_Struct( obj, struct ofp_match, match );

    memcpy(match->dl_dst, p->match.dl_dst, OFP_ETH_ALEN);
    memcpy(match->dl_src, p->match.dl_src, OFP_ETH_ALEN);
    match->dl_type = p->match.dl_type;
    match->dl_vlan = p->match.dl_vlan;
    match->dl_vlan_pcp = p->match.dl_vlan_pcp;
    match->in_port = p->match.in_port;
    match->nw_dst = p->match.nw_dst;
    match->nw_proto = p->match.nw_proto;
    match->nw_src = p->match.nw_src;
    match->nw_tos = p->match.nw_tos;
    match->tp_dst = p->match.tp_dst;
    match->tp_src = p->match.tp_src;
    match->wildcards = p->match.wildcards;

    debug("end\n");
    return obj;
}

/*
 * Start setting up the path.
 *
 * @overload setup(controller)
 *
 * @param [Controller]
 *
 * @return [Bool]
 */
static VALUE path_setup(VALUE self, VALUE controller)
{
    debug("start\n");

    VALUE ret;

    if ( rb_respond_to( ( VALUE ) mFlowManager, rb_intern( "setup" ) ) == Qtrue ) {
      ret = rb_funcall( ( VALUE ) mFlowManager, rb_intern( "setup" ), 2, self, controller);
    }

    if(ret == Qtrue)
    {
    	return Qtrue;
    }
    else
    {
    	return Qfalse;
    }
}

/*
 * Teardown the path
 *
 * @return [Bool]
 */
static VALUE path_terdown(VALUE self)
{
    debug("start\n");

    path *p;
    Data_Get_Struct(self, path, p);

    struct ofp_match _match = p->match;

    list_element *element = p->hops;

    if ( element == NULL ) {
    	rb_raise( rb_eTypeError, "actions argument must be an Array or an Action object" );
    }

    hop_private *temp = (hop_private *)element->data;
    uint64_t _in_datapath_id = temp->public.datapath_id;
    uint16_t _priority = p->priority;

    bool ret = teardown_path(_in_datapath_id, _match, _priority );
    debug("end with %d\n", ret);

    if(ret == 1)
    {
            return Qtrue;
    }
    else
    {
            return Qfalse;
    }
}

/*
 * Append a hop to a path
 *
 * @param [Hop] hop.
 *
 * @return [Nil] nil.
 */
static VALUE path_append_hop(VALUE self, VALUE rhop)
{
	debug("start\n");

	if ( rb_funcall( rhop, rb_intern( "is_a?" ), 1, cHop ) == Qfalse )
	{
        rb_raise( rb_eTypeError, "Argument should be Hop class");
	}

    path *p;
    hop *h;

    Data_Get_Struct(self, path, p);
    Data_Get_Struct(rhop, hop, h);

    append_hop_to_path(p, h);

    debug("end\n");
	return Qnil;
}

/*
 * Append hops to a path.
 *
 * @overload append_hops(hops)
 *
 * @param [Array] hops
 *
 * @return [Nil] nil.
 */
static VALUE path_append_hops(VALUE self, VALUE rhops)
{
    debug("start\n");
    UNUSED( self );

    path *p;

    Data_Get_Struct(self, path, p);

    if ( rhops != Qnil ) {
      debug("type : %d\n", TYPE(rhops));
      switch ( TYPE( rhops ) ) {
        case T_ARRAY:
          {
            VALUE *each = RARRAY_PTR( rhops );
            int i;
            for ( i = 0; i < RARRAY_LEN( rhops ); i++ ) {
                          hop *h;
                          Data_Get_Struct(each[i], hop, h);
                          append_hop_to_path(p, h);
            }
          }
          break;
        default:
          debug("start7\n");
          rb_raise( rb_eTypeError, "Argument should be an Array" );
          break;
      }
    }

    debug("end_\n");
    return Qnil;
}

/*
 * Creates a {Path} instance.
 *
 * @overload initialize(match, options={})
 *
 *   @example
 *     Path.new(
 *       Match match,
 *       :priority => priority,
 *       :idle_timeout => idle_timeout,
 *       :hard_timeout => hard_timeout,
 *     )
 *
 *   @param [Hash] options the options hash.
 *
 *   @param [Match] match match.
 *
 *   @option options [Number] :priority the priority for the path.
 *
 *   @option options [Number] :idle_timeout the idle_timeout for the path.
 *
 *   @option options [Number] :hard_timeout the hard_timeout for the path.
 *
 *   @return [Path] self an object that encapsulates and wraps the +path+
 */
static VALUE path_initialize(int argc, VALUE *argv, VALUE self)
{
    debug("start\n");
    path *p;
    Data_Get_Struct(self, path, p);

    struct ofp_match *_match;
    VALUE options;
    VALUE match;

    int nargs = rb_scan_args(argc, argv, "11", &match, &options);
    Data_Get_Struct(match, struct ofp_match, _match );
    p->match = (struct ofp_match)*_match;
    debug(dump_match(_match));

    switch(nargs)
    {
        case 1:
        {
            //Do nothing
            break;
        }
        case 2:
        {
            if(TYPE( options ) != T_HASH)
            {
                rb_raise( rb_eTypeError, "The second argument should be hash." );
            }

            if(options != Qnil)
            {
                VALUE priority = rb_hash_aref(options, ID2SYM( rb_intern( "priority" ) ) );
                if ( priority != Qnil ) {

                    if(NUM2INT(priority)<0)
                    {
                      rb_raise( rb_eRangeError, "Please input positive integer" );
                    }

                    p->priority = (uint16_t)NUM2INT( priority );
                }

                VALUE idle_timeout = rb_hash_aref(options, ID2SYM( rb_intern( "idle_timeout" ) ) );
                if ( idle_timeout != Qnil ) {

                    if(NUM2INT(idle_timeout)<0)
                    {
                      rb_raise( rb_eRangeError, "Please input positive integer" );
                    }

                    p->idle_timeout = (uint16_t)NUM2INT( idle_timeout );
                }

                VALUE hard_timeout = rb_hash_aref(options, ID2SYM( rb_intern( "hard_timeout" ) ) );
                if ( hard_timeout != Qnil ) {

                    if(NUM2INT(hard_timeout)<0)
                    {
                      rb_raise( rb_eRangeError, "Please input positive integer" );
                    }

                    p->hard_timeout = (uint16_t)NUM2INT( hard_timeout );
                }
            }
            break;
        }
        default:
        {
            rb_raise( rb_eArgError, "The number of argument is invalid." );
            break;
        }
    }

    debug("end\n");
    return Qnil;
}

static VALUE create_Path(VALUE klass)
{
    debug("start\n");
    path_private *pp = ALLOC( path_private );
    memset( pp, 0, sizeof( path_private ) );
    debug("pass private %p is created.\n", pp);

    //default value
    pp->public.priority = 65535;
    pp->public.idle_timeout = 30;
    pp->public.hard_timeout = 30;
    pp->public.n_hops = 0;
    create_list( &pp->public.hops );

    path *p = &pp->public;
    debug("pass %p.\n", p);
    VALUE rPath = Data_Wrap_Struct(klass, 0, -1, p);
    debug("ruby pass %p is created.\n", rPath);
    debug("end\n");

    return rPath;
}

/*
 * Document-class: Trema::Path
 */
void Init_path()
{
	cPath = rb_define_class_under(mTrema, "Path", rb_cObject);
	rb_define_alloc_func(cPath, create_Path);
	rb_define_method(cPath, "initialize", path_initialize, -1);
	rb_define_method(cPath, "priority", path_priority, 0);
	rb_define_method(cPath, "idle_timeout", path_idle_timeout, 0);
	rb_define_method(cPath, "hard_timeout", path_hard_timeout, 0);
	rb_define_method(cPath, "match", path_match, 0);
	rb_define_method(cPath, "hops", path_hops, 0);
	rb_define_method(cPath, "<<", path_append_hop, 1);
	rb_define_method(cPath, "append_hop", path_append_hop, 1);
	rb_define_method(cPath, "append_hops", path_append_hops, 1);
	rb_define_method(cPath, "teardown", path_terdown, 0);
	rb_define_method(cPath, "setup", path_setup,1);
}
