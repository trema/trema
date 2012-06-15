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


require "forwardable"


module Trema
  #
  # MAC address class
  #
  class Mac
    extend Forwardable
    def_delegator :@value, :hash


    #
    # @return [Number] Ethernet address in its numeric presentation.
    #
    attr_reader :value


    #
    # Creates a {Mac} instance that encapsulates Ethernet MAC addresses.
    #
    # @overload initialize(value)
    #
    # @param [String,Integer] value
    #   the MAC address to set to.
    #
    # @example address as a hexadecimal string
    #   Mac.new("11:22:33:44:55:66")
    #
    # @example address as a hexadecimal number
    #   Mac.new(0xffffffffffff)
    #
    # @raise [ArgumentError] if invalid format is detected.
    # @raise [ArgumentError] if supplied argument is not a string or integer.
    #
    def initialize value
      case value
      when String
        @value = from_string( value )
      when Integer
        @value = from_integer( value )
      else
        raise %{Invalid MAC address: #{ value.inspect }}
      end
      @string = string_format
    end


    #
    # @return [String]
    #  the Ethernet address as 6 pairs of hexadecimal digits delimited by colons.
    #  eg. xx:xx:xx:xx:xx:xx
    #
    def to_s
      @string
    end


    #
    # @return [Array]
    #   an array of decimal numbers converted from Ethernet's address string
    #   format.
    #
    def to_short
      @string.split( ":" ).collect do | each |
        each.hex
      end
    end


    #
    # @return [Array]
    #   an array of decimal numbers converted from Ethernet's address string
    #   format.
    #
    def to_array
      self.to_short
    end


    #
    # @return [Boolean] if other matches or not the attribute type value.
    #
    def == other
      @value == other.value
    end


    #
    # @return [Boolean] if other matches or not the attribute type value.
    #
    def eql? other
      @value == other.value
    end


    #
    # @return [Boolean] if MAC address is multicast or not.
    #
    def is_multicast?
      # check I/G bit
      return to_short[0] & 1 == 1
    end


    ################################################################################
    private
    ################################################################################


    def from_string string
      octet_regex = "[0-9a-fA-F][0-9a-fA-F]"
      if /^(#{ octet_regex }:){5}(#{ octet_regex })$/=~ string
        eval( "0x" + string.gsub( ":", "" ) )
      else
        raise %{Invalid MAC address: "#{ string }"}
      end
    end


    def from_integer integer
      if integer >= 0 and integer <= 0xffffffffffff
        integer
      else
        raise %{Invalid MAC address: #{ integer }}
      end
    end


    def string_format
      sprintf( "%012x", @value ).unpack( "a2" * 6 ).join( ":" )
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
