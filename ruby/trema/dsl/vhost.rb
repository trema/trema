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

require 'trema/dsl/stanza'
require 'trema/dsl/syntax-error'

module Trema
  module DSL
    #
    # The syntax definition of vhost { ... } stanza in Trema DSL.
    #
    class Vhost < Stanza
      #
      # Set promisc mode on/off
      #
      # @example
      #   stanza.promisc "on"
      #
      # @return [undefined]
      #
      # @api public
      #
      def promisc(on_off)
        case on_off.to_s.downcase
        when 'on', 'yes'
          @promisc = true
        when 'off', 'no'
          @promisc = false
        else
          fail SyntaxError, "Unknown option: promisc #{ on_off }"
        end
      end

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
      def ip(str)
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
      def netmask(str)
        @netmask = str
      end

      #
      # Set MAC address
      #
      # @example
      #   stanza.mac "00:00:00:01:00:01"
      #
      # @return [undefined]
      #
      # @api public
      #
      def mac(str)
        @mac = str
      end
    end
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
