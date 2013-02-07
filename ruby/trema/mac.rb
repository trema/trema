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


require "forwardable"


module Trema
  #
  # Ethernet address class
  #
  class Mac
    extend Forwardable
    def_delegator :@value, :hash


    #
    # Returns an Ethernet address in its numeric presentation.
    #
    # @example
    #   Mac.new("11:22:33:44:55:66") #=> 18838586676582
    #
    # @return [Number] the Ethernet address in numeric format
    #
    attr_reader :value


    #
    # Creates a {Mac} instance that encapsulates Ethernet addresses.
    #
    # @overload initialize(value)
    #
    # @param [String,Integer] value
    #   the Ethernet address to set to.
    #
    # @example address as a hexadecimal string
    #   Mac.new("11:22:33:44:55:66")
    #
    # @example address as a hexadecimal number
    #   Mac.new(0xffffffffffff)
    #
    # @raise [ArgumentError] if invalid format is detected.
    # @raise [TypeError] if supplied argument is not a String or Integer.
    #
    def initialize value
      case value
        when String
          @value = create_from( value )
        when Integer
          @value = value
          validate_value_range
        when Mac
          @value = create_from( value.to_s )
        else
          raise TypeError, "Invalid MAC address: #{ value.inspect }"
      end
      @string = string_format
    end


    #
    # Returns the Ethernet address as 6 pairs of hexadecimal digits
    # delimited by colons.
    #
    # @example
    #   Mac.new(18838586676582).to_s #=> "11:22:33:44:55:66"
    #
    # @return [String] the Ethernet address in String format
    #
    def to_s
      @string
    end


    #
    # Returns an array of decimal numbers converted from Ethernet's
    # address string format.
    #
    # @example
    #   Mac.new("11:22:33:44:55:66").to_a #=> [ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 ]
    #
    # @return [Array] the Ethernet address in Array format
    #
    def to_a
      @string.split( ":" ).collect do | each |
        each.hex
      end
    end


    #
    # @private
    #
    def == other
      @value == other.value
    end
    alias :eql? :==


    #
    # Returns true if Ethernet address is a multicast address.
    #
    # @example
    #   Mac.new("01:00:00:00:00:00").multicast? #=> true
    #   Mac.new("00:00:00:00:00:00").multicast? #=> false
    #
    # @return [Boolean] whether the Ethernet address is multicast
    #
    def multicast?
      to_a[ 0 ] & 1 == 1
    end


    #
    # Returns true if Ethernet address is a broadcast address.
    #
    # @example
    #   Mac.new("ff:ff:ff:ff:ff:ff").broadcast? #=> true
    #
    # @return [Boolean] whether the Ethernet address is broadcast
    #
    def broadcast?
      to_a.all? { | each | each == 0xff }
    end


    ################################################################################
    private
    ################################################################################


    def create_from string
      octet_regex = "[0-9a-fA-F][0-9a-fA-F]"
      if /^(#{ octet_regex }:){5}(#{ octet_regex })$/=~ string
        string.gsub( ":", "" ).hex
      else
        raise ArgumentError, %{Invalid MAC address: "#{ string }"}
      end
    end


    def validate_value_range
      unless ( @value >= 0 and @value <= 0xffffffffffff )
        raise ArgumentError, "Invalid MAC address: #{ @value }"
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
