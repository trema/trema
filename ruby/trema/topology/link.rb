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
    # Link's Hash key 4-tuple's array index
    FROM_DPID = 0
    # Link's Hash key 4-tuple's array index
    FROM_PORTNO = 1
    # Link's Hash key 4-tuple's array index
    TO_DPID = 2
    # Link's Hash key 4-tuple's array index
    TO_PORTNO = 3

    # A class to represent a link in a Topology
    class Link

      # @!attribute [r] from_dpid
      # @return [Integer] datapath ID of the switch which this link departs from
      def from_dpid
        return @property[:from_dpid]
      end


      # @!attribute [r] from_portno
      # @return [Integer] port number which this link departs from
      def from_portno
        return @property[:from_portno]
      end


      # @!attribute [r] to_dpid
      # @return [Integer] datapath ID of the switch which this link arrive to
      def to_dpid
        return @property[:to_dpid]
      end


      # @!attribute [r] to_portno
      # @return [Integer] port number which this link departs arrive to
      def to_portno
        return @property[:to_portno]
      end


      # @return [Boolean] returns true if link is up
      def up?
        return @property[:up]
      end


      # @return [Boolean] returns true if link is unstable
      def unstable?
        return @property[:unstable]
      end


      # @return [[Integer,Integer,Integer,Integer]] Link key 4-tuple for this Link instance
      def key
        return [ from_dpid, from_portno, to_dpid, to_portno ]
      end


      # Link constructor.
      # @param [Hash] link Hash containing Link properties. Must at least contain keys listed in Options.
      # @option link [Integer] :from_dpid Switch dpid which this link departs from
      # @option link [Integer] :from_portno port number of switch which this link departs from
      # @option link [Integer] :to_dpid Switch dpid which this link peer to
      # @option link [Integer] :to_portno port number of switch which this link peer to
      # @example
      #   link = Link.new( {:from_dpid => 0x1234, :from_portno => 42, :to_dpid => 0x5678, :to_portno => 72 } )
      def initialize( link )
        raise ArgumentError, "Mandatory Key element for Link missing in Hash" if not Link.has_mandatory_keys?( link )
        # TODO type check mandatory key?
        # TODO copy Hash?
        @property = link
      end


      # @param [Symbol] key Hash key element
      # @return [Boolean] true if key is key element for Link
      def Link.is_mandatory_key?( key )
        case key
        when :from_dpid, :from_portno, :to_dpid, :to_portno
          return true
        else
          return false
        end
      end


      # Test if Hash has required key as a Link instance
      # @param [Hash] hash Hash to test
      # @return [Boolean] true if hash has all required keys.
      def Link.has_mandatory_keys?( hash )
        return !(hash.values_at(:from_dpid, :from_portno, :to_dpid, :to_portno).include? nil)
      end


      # @return [String] human readable string representation.
      def to_s
        "Link: (0x#{ from_dpid.to_s(16) }:#{ from_portno.to_s })->(0x#{ to_dpid.to_s(16) }:#{ to_portno.to_s }) - {#{property_to_s}}"
      end
      
      
      ############################################################################
      protected
      ############################################################################


      def property_to_s
        kvp_ary = @property.select { |key,_| not Link.is_mandatory_key?( key ) }. 
          map { |key, val| [key.to_s, val] }. 
          sort { |lhs,rhs| lhs.first <=> rhs.first }. 
          map { |key, val| "#{key}:#{val.inspect}"} 
        return kvp_ary.join(", ")
      end
    end
  end
end
