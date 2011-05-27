#
# DSL parser.
#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
#
# Copyright (C) 2008-2011 NEC Corporation
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2, as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#


require "fileutils"
require "trema/dsl/syntax"
require "trema/path"


module Trema
  module DSL
    class Parser
      CURRENT_CONTEXT = File.join( Trema.tmp, ".context" )


      def self.dump method
        Syntax.module_eval do
          original = instance_method( method )
          define_method( method ) do | *args, &block |
            if block
              original.bind( self ).call( *args, &block )
            else
              original.bind( self ).call( *args )
            end
            instance_variable_get( :@context ).dump_to CURRENT_CONTEXT
          end
        end
      end


      dump :app
      dump :event
      dump :filter
      dump :link
      dump :port
      dump :switch
      dump :use_tremashark
      dump :vhost
      dump :vswitch


      def load_current
        Context.load_from CURRENT_CONTEXT
      end


      def parse file_name
        new_context do | context |
          Syntax.new( context ).instance_eval IO.read( file_name ), file_name
        end
      end


      def eval &block
        new_context do | context |
          Syntax.new( context ).instance_eval &block
        end
      end


      ################################################################################
      private
      ################################################################################


      def new_context &block
        context = Context.new
        block.call context
        Trema::Link.each do | each |
          peers = each.peers
          Trema::Host[ peers[ 0 ] ].interface = each.interfaces[ 0 ] if Trema::Host[ peers[ 0 ] ]
          Trema::Host[ peers[ 1 ] ].interface = each.interfaces[ 1 ] if Trema::Host[ peers[ 1 ] ]
          Trema::Switch[ peers[ 0 ] ].add_interface each.interfaces[ 0 ] if Trema::Switch[ peers[ 0 ] ]
          Trema::Switch[ peers[ 1 ] ].add_interface each.interfaces[ 1 ] if Trema::Switch[ peers[ 1 ] ]
        end
        context.dump_to CURRENT_CONTEXT
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
