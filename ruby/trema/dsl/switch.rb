#
# The syntax definition of switch { ... } stanza in Trema DSL.
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


require "trema/dsl/stanza"


module Trema
  module DSL
    class Switch < Stanza
      def dpid str
        no_0x = str.gsub( /^0x/, "" )
        @dpid_long = "0" * ( 16 - no_0x.length ) + no_0x
        @dpid_short = str
        if @name.nil?
          @name = @dpid_short
        end
      end
      alias :datapath_id :dpid


      def ports str
        @ports = str
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
