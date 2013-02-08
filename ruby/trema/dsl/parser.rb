#
# DSL parser.
#
# Copyright (C) 2008-2013 NEC Corporation
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


require "trema/dsl/context"
require "trema/dsl/syntax"


module Trema
  module DSL
    class Parser
      def parse file_name
        configure do | config |
          Syntax.new( config ).instance_eval IO.read( file_name ), file_name
        end
      end


      def eval &block
        configure do | config |
          Syntax.new( config ).instance_eval &block
        end
      end


      ################################################################################
      private
      ################################################################################


      def configure &block
        config = Configuration.new
        block.call config
        Trema::Link.each do | each |
          peers = each.peers
          config.hosts[ peers[ 0 ] ].interface = each.name if config.hosts[ peers[ 0 ] ]
          config.hosts[ peers[ 1 ] ].interface = each.name_peer if config.hosts[ peers[ 1 ] ]
          config.netnss[ peers[ 0 ] ].interface = each.name if config.netnss[ peers[ 0 ] ]
          config.netnss[ peers[ 1 ] ].interface = each.name_peer if config.netnss[ peers[ 1 ] ]
          config.switches[ peers[ 0 ] ] << each.name if config.switches[ peers[ 0 ] ]
          config.switches[ peers[ 1 ] ] << each.name_peer if config.switches[ peers[ 1 ] ]
        end
        Context.new( config ).dump
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
