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


require "trema/executables"
require "trema/network-component"


module Trema
  #
  # The controller class of packetin_filter.
  #
  class PacketinFilter < NetworkComponent
    #
    # Creates a PacketinFilter controller
    #
    # @example
    #   Trema::PacketinFilter.new( :lldp => "topology manager", :packet_in => "OpenFlow ping-pong" )
    #
    # @return [PacketinFilter]
    #
    # @api public
    #
    def initialize queues
      check_mandatory_options queues
      @queues = queues
      PacketinFilter.add self
    end


    #
    # Returns the name of packetin filter
    #
    # @example
    #   packetin_filter.name => "packet-in filter"
    #
    # @return [String]
    #
    # @api public
    #
    def name
      "packet-in filter"
    end


    #
    # Starts a packetin filter process
    #
    # @example
    #   packetin_filter.run!
    #
    # @return [undefined]
    #
    # @api public
    #
    def run!
      sh "#{ Executables.packetin_filter } --daemonize --name=filter #{ lldp_queue } #{ packetin_queue }"
    end


    ################################################################################
    private
    ################################################################################


    #
    # Checks mandatory options for packetin_filter command
    #
    # @return [undefined]
    #
    # @api private
    #
    def check_mandatory_options queues
      [ :lldp, :packet_in ].each do | each |
        raise ":#{ each } is a mandatory option" if queues[ each ].nil?
      end
    end


    #
    # The lldp option of packetin_filter command
    #
    # @return [String]
    #
    # @api private
    #
    def lldp_queue
      "lldp::#{ @queues[ :lldp ] }"
    end


    #
    # The packetin option of packetin_filter command
    #
    # @return [String]
    #
    # @api private
    #
    def packetin_queue
      "packet_in::#{ @queues[ :packet_in ] }"
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
