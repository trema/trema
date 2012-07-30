#
# Copyright (C) 2008-2012 NEC Corporation
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


require "trema/action"


module Trema
  #
  # An action to modify the IPv4 source address of a packet.
  #
  class ActionSetNwSrc < Action
    attr_reader :ip_address


    #
    # Creates an action to modify the IPv4 source address of a
    # packet.
    #
    # @example
    #   ActionSetNwSrc.new("192.168.1.1")
    #
    # @param [String] ip_address
    #   the IPv4 address to create this action with.
    #
    # @raise [TypeError] if ip_address is not an String.
    #
    def initialize ip_address
      if not ip_address.is_a?( String )
        raise TypeError, "Source IP address must be an String"
      end
      @ip_address = IPAddr.new( ip_address )
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
