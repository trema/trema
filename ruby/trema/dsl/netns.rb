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
require "trema/dsl/syntax-error"


module Trema
  module DSL
    #
    # The syntax definition of netns { ... } stanza in Trema DSL.
    #
    class Netns < Stanza
      #
      # Set IP address
      #
      # @example
      #   stanza.ip "192.168.100.1"
      #
      # @return [undefined]
      #
      # @api public
      #
      def ip str
        @ip = str
        if @name.nil?
          @name = @ip
        end
      end


      #
      # Set netmask
      #
      # @example
      #   stanza.netmask "255.255.0.0"
      #
      # @return [undefined]
      #
      # @api public
      #
      def netmask str
        @netmask = str
      end


      def route options
        if options[ :net ].nil? or options[ :gw ].nil?
          raise ":net and :gw option is a mandatory"
        end
        @net = options[ :net ]
        @gw = options[ :gw ]
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
