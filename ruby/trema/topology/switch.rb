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
    # A class to represent a switch in a Topology
    class Switch

      # @return [{Integer=>Port}] Hash of Ports: port_no => Port
      attr_reader :ports


      # @return [{[Integer,Integer,Integer,Integer]=>Link}] Hash of inbound/outbound Link: [from.dpid, from.port_no, to.dpid, to.port_no] => Links
      attr_reader :links_in, :links_out


      # @!attribute [r] links
      # @return [{[Integer,Integer,Integer,Integer]=>Link}] Hash of inbound+outbound Link: [from.dpid, from.port_no, to.dpid, to.port_no] => Links
      def links
        return links_in.merge(links_out)
      end


      # @!attribute [r] dpid
      # @return [Integer] Switch datapath ID for this Switch instance
      def dpid
        return @property[:dpid]
      end


      # @return [Boolean] returns true if switch is up
      def up?
        return @property[:up]
      end


      # @return [Integer] Switch key 4-tuple for this Switch instance
      def key
        return dpid
      end


      # Add a port on this Switch.
      # @param [Port] port Port instance to add to switch
      # @return self
      def add_port port
        raise TypeError, "Trema::Topology::Port expected" if not port.is_a?(Port)
        raise ArgumentError, "dpid mismatch. 0x#{ self.dpid.to_s(16) } expected but received: 0x#{ port.dpid.to_s(16) }" if ( self.dpid != port.dpid )
        @ports[ port.portno ] = port
          return self
      end


      # Delete a port on this Switch.
      # It will be silently ignored if the port did not belong to this switch
      # @overload delete_port port
      #   @param [Port] port Port instance to delete
      #   @return self
      # @overload delete_port port_hash
      #   @param [Hash] port_hash a Hash with port info about instance to delete
      #   @return self
      # @overload delete_port portno
      #   @param [Integer] portno portno of instance to delete
      #   @return self
      def delete_port port
        if port.is_a?(Port)
          @ports.delete( port.portno )
        elsif port.is_a(Hash)
          @ports.delete( port[:portno] )
        elsif port.is_a(Integer)
          @ports.delete( port )
        else
          raise TypeError, "Either Port, Port info(Hash), portno(Integer) expected"
        end
        return self
      end


      # Add a link to this Switch.
      # It will be silently ignored if the link was not bound to this switch
      # @param [Link] link link to add.
      # @return self
      def add_link link
        @links_in[ link.key ] = link if link.to_dpid == self.dpid
        @links_out[ link.key ] = link if link.from_dpid == self.dpid
        return self
      end


      # Delete a link on this Switch.
      # It will be silently ignored if the link was not bound to this switch
      # @overload delete_link link
      #   @param [Port] port Link instance to delete
      #   @return self
      # @overload delete_link link_hash
      #   @param [Hash] port_hash a Hash with link info about instance to delete
      #   @return self
      # @overload delete_link key
      #   @param [[Integer, Integer, Integer, Integer]] key Link key 4-tuple of the link to delete
      #   @return self
      def delete_link link
        if link.is_a?(Link)
          key = link.key
          @links_in.delete( key ) if link.to_dpid == self.dpid
          @links_out.delete( key ) if link.from_dpid == self.dpid
        elsif link.is_a?(Hash)
          key = [ link[:from_dpid], link[:from_portno], link[:to_dpid], link[:to_portno] ]
          @links_in.delete( key ) if link[:to_dpid] == self.dpid
          @links_out.delete( key ) if link[:from_dpid] == self.dpid
        elsif link.is_a?(Array) and link.size == 4
          key = [ link[FROM_DPID], link[FROM_PORTNO], link[TO_DPID], link[TO_PORTNO] ]
          @links_in.delete( key ) if link[TO_DPID] == self.dpid
          @links_out.delete( key ) if link[FROM_DPID] == self.dpid
        else
          raise TypeError, "Either Link, Link info(Hash), Link key tuple([Integer,Integer,Integer,Integer]) expected"
        end
        return self
      end


      # Switch constructor.
      # @overload initialize sw_hash
      #   @param [Hash] sw_hash a Hash containing Switch properties. Must at least contain keys listed in Options.
      #   @option sw_hash [Integer] :dpid Switch dpid
      # @overload initialize dpid
      #   @param [Integer] dpid datapath_id  
      # @example
      #  sw = Switch.new( {:dpid => 0x1234} )
      #  sw = Switch.new( 0x1234 )
      def initialize( sw )
        sw = { :dpid => sw, :up => true } if sw.is_a?(Integer)
        raise ArgumentError, "Mandatory key element for Switch missing in Hash" if not Switch.has_mandatory_keys?( sw )
        sw.merge!( { :up => true } ) if not sw.member? :up

        # TODO type check mandatory key?
        # TODO copy Hash?
        @property = sw
        @ports = Hash.new
        @links_in = Hash.new
        @links_out = Hash.new
      end


      # @param [Symbol] key Hash key element
      # @return [Boolean] true if key is key element for Switch
      def Switch.is_mandatory_key?( key )
        return ( key == :dpid )
      end


      # Test if Hash has required key as a Switch instance
      # @param hash Hash to test
      # @return [Boolean] true if hash has all required keys.
      def Switch.has_mandatory_keys?( hash )
        return hash.has_key?( :dpid )
      end


      # @return [String] human readable string representation.
      def to_s
        s = "Switch: 0x#{ dpid.to_s(16) } - {#{property_to_s}}\n"
        @ports.each_pair do |_,port|
          s << " #{ port.to_s }\n"
        end
        s << " Links_in\n" if not @links_in.empty?
        @links_in.each_pair do |key,_|
          s << "  <= 0x#{ key[FROM_DPID].to_s(16) }:#{ key[FROM_PORTNO] }\n"
        end
        s << " Links_out\n" if not @links_out.empty?
        @links_out.each_pair do |key,_|
          s << "  => 0x#{ key[ TO_DPID ].to_s(16) }:#{ key[ TO_PORTNO ] }\n"
        end
        return s
      end


      ############################################################################
      protected
      ############################################################################


      def property_to_s
        kvp_ary = @property.select { |key,_| not Switch.is_mandatory_key?( key ) }. 
          map { |key, val| [key.to_s, val] }. 
          sort { |lhs,rhs| lhs.first <=> rhs.first }. 
          map { |key, val| "#{key}:#{val.inspect}"} 
        return kvp_ary.join(", ")
      end
    end
  end
end

