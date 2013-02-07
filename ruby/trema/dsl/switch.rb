#
# The syntax definition of switch { ... } stanza in Trema DSL.
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


require "trema/dsl/stanza"


module Trema
  module DSL
    class Switch < Stanza
      def dpid value
        case value
        when String
          set_dpid value
        when Integer
          set_dpid sprintf( "%#x", value )
        else
          raise "Invalid datapath_id: #{ value }"
        end
      end
      alias :datapath_id :dpid


      def ports str
        @ports = str
      end


      def validate
        set_dpid name if /\A0x/=~ name
        raise "Invalid dpid: #{ @name }" if @dpid_short.nil?
      end


      ##########################################################################
      private
      ##########################################################################


      def set_dpid string
        @dpid_long = dpid_long_from( string )
        @dpid_short = string
        @name = @dpid_short if @name.nil?
      end


      def dpid_long_from string
        no_0x = string.gsub( /^0x/, "" )
        "0" * ( 16 - no_0x.length ) + no_0x
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
