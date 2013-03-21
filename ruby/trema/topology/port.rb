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
  module Topology
    # A class to represent a port in a Topology
    class Port

      # @!attribute [r] dpid
      # @return [Integer] datapath ID of the switch which this port belong to.
      def dpid
        return @property[:dpid]
      end


      # @!attribute [r] portno
      # @return [Integer] port number
      def portno
        return @property[:portno]
      end


      # @!attribute [r] up
      # @return [Boolean] returns true if port is up
      def up?
        return @property[:up]
      end


      # @return [Boolean] returns true if port is external
      def external?
        return @property[:external]
      end


      # @!attribute [r] name
      # @return [String] returns port's name
      def name
        return @property[:name]
      end


      # @!attribute [r] mac
      # @return [String] returns port's mac address
      def mac
        return @property[:mac]
      end


      # @return [[Integer,Integer]] Port key 2-tuple for this Port instance
      def key
        return [dpid, portno]
      end


      # Port constructor
      # @param [Hash] port Hash containing Port properties. Must at least contain keys listed in Options.
      # @option port [Integer] :dpid Switch dpid which this port belongs
      # @option port [Integer] :portno port number
      # @example
      #   port = Port.new( {:dpid => 1234, :portno => 42} )
      def initialize( port )
        raise ArgumentError, "Mandatory key element for Port missing in Hash" if not Port.has_mandatory_keys?( port )
        # TODO type check mandatory key?
        # TODO copy Hash?
        @property = port
      end


      # @param [Symbol] key Hash key element
      # @return [Boolean] true if key is key element for Port
      def Port.is_mandatory_key?( key )
        case key
        when :dpid, :portno
          return true
        else
          return false
        end
      end


      # Test if Hash has required key as a Port instance
      # @param hash Hash to test
      # @return [Boolean] true if hash has all required keys.
      def Port.has_mandatory_keys?( hash )
        return !(hash.values_at(:dpid, :portno).include? nil)
      end


      # @return [String] human readable string representation.
      def to_s
        "Port: 0x#{ dpid.to_s(16) }:#{ portno.to_s } - {#{property_to_s}}"
      end


      ############################################################################
      protected
      ############################################################################


      def property_to_s
        kvp_ary = @property.select { |key,_| not Port.is_mandatory_key?( key ) }. 
          map { |key, val| [key.to_s, val] }. 
          sort { |lhs,rhs| lhs.first <=> rhs.first }. 
          map { |key, val| "#{key}:#{val.inspect}"} 
        return kvp_ary.join(", ")
      end
    end
  end
end
