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


module Trema
  #
  # A wrapper class to IPAddr
  #
  class IP
    require "ipaddr"


    #
    # @return [IPAddr] value object instance of proxied IPAddr.
    #
    attr_reader :value


    #
    # Creates a {IP} instance object as a proxy to IPAddr class.
    #
    # @overload initialize(addr)
    #
    # @param [String, Number] addr
    #   an IPv4 address specified either as a String or Number.
    #
    # @param [Number] prefixlen
    #   masking IPv4 address with given prefixlen.
    #
    # @raise [ArgumentError] invalid address if supplied argument is invalid
    #   IPv4 address.
    #
    # @return [IP] self
    #   a proxy to IPAddr.
    #
    def initialize addr, prefixlen = 32
      if !addr.kind_of? String
        @value = IPAddr.new( addr, Socket::AF_INET )
      else
        @value = IPAddr.new( addr )
      end
      if prefixlen < 32
        @value = @value.mask( prefixlen )
      end
    end


    #
    # @return [ Number ] prefixlen of IPv4 address.
    #
    def prefixlen
      range = @value.to_range
      mask = range.first.to_i ^ range.last.to_i
      masklen = 0
      while mask != 0 do
        mask = mask >> 1
        masklen += 1
      end
      return 32 - masklen
    end


    #
    # @return [String] the IPv4 address in its text representation.
    #
    def to_s
      @value.to_s
    end


    #
    # @return [Number] the IPv4 address in its numeric representation.
    #
    def to_i
      @value.to_i
    end


    #
    # @return [Array]
    #    an array of decimal numbers converted from IP address.
    #
    def to_a
      to_s.split( "." ).collect do | each |
        each.to_i
      end
    end
    alias :to_array :to_a
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
