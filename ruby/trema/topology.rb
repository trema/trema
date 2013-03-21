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

require 'trema/topology/link'
require 'trema/topology/port'
require 'trema/topology/switch'


module Trema
  # A module to add topology information notification capability to Controller
  module Topology
    #
    # @private Just a placeholder for YARD.
    #
    def self.handler name
      # Do nothing.
    end


    # @!group Topology update event handlers

    #
    #
    # @!method switch_status_updated( sw_stat )
    # Switch status update event handler.
    # @abstract
    # @note In general user should override #switch_status_up and #switch_status_down,
    #       unless user have common operation for both events.
    #
    # @note sw_stat contains info about switch attribute only. It will not contain info about it's ports.
    #
    # @param [Hash] sw_stat
    #   Hash containing info about updated switch.
    # @option sw_stat [Integer] :dpid dpid of the switch
    # @option sw_stat [Boolean] :up true if status is TD_SWITCH_UP
    # @return [void]
    #
    handler :switch_status_updated


    #
    # @overload switch_status_up( dpid )
    # Switch status up event handler.
    # @abstract
    # @param [Integer] dpid datapath_id of the switch.
    # @return [void]
    #
    handler :switch_status_up


    #
    # @overload switch_status_down( dpid )
    # Switch status down event handler.
    # @abstract
    # @param [Integer] dpid datapath_id of the switch.
    # @return [void]
    #
    handler :switch_status_down

    #
    # @!method port_status_updated( port_stat )
    #
    # @abstract port_status_updated event handler. Override this to implement a custom handler.
    # Port status update event handler.
    #
    # @param [Hash] port_stat
    #   Hash containing info about updated port.
    # @option port_stat [Integer] :dpid dpid of the switch
    # @option port_stat [Integer] :portno port number. Note that attribute name differ from C structs.
    # @option port_stat [String] :name name of the port
    # @option port_stat [String] :mac mac address
    # @option port_stat [Integer] :external external flag of the port. Refer to enum topology_port_external_type in topology_service_interface.h
    # @option port_stat [Boolean] :up true if status is TD_PORT_UP
    # @return [void]
    #
    handler :port_status_updated

    #
    # @!method link_status_updated( link_stat )
    #
    # @abstract link_status_updated event handler. Override this to implement a custom handler.
    # Link status update event handler.
    #
    # @param [Hash] link_stat
    #   Hash containing info about updated link.
    # @option link_stat [Integer] :from_dpid dpid of the switch which the link departs
    # @option link_stat [Integer] :from_portno port number of the switch which the link departs
    # @option link_stat [Integer] :to_dpid dpid of the switch which the link arraives
    # @option link_stat [Integer] :to_portno port number of the switch which the link arrives
    # @option link_stat [Boolean] :up true if status is *NOT* TD_LINK_DOWN, false otherwise.
    # @option link_stat [Boolean] :unstable true if status is TD_LINK_UNSTABLE, false otherwise.
    # @return [void]
    #
    handler :link_status_updated


    # @!group Get all status request/reply

    #
    # @!method all_switch_status( sw_stats )
    # get_all_switch_status reply event handler,
    # when handler block was omitted on get_all_switch_status call.
    # @abstract
    #
    # @param [Array<Hash>] sw_stats
    #   Array of Hash containing info about updated switch.
    # @see #switch_status_updated Each Hash instance included in the array is equivalent to #switch_status_updated argument Hash.
    # @return [void]
    #
    handler :all_switch_status

    #
    # @!method all_port_status( port_stats )
    # get_all_port_status reply event handler,
    # when handler block was omitted on get_all_port_status call.
    # @abstract
    #
    # @param [Array<Hash>] port_stats
    #   Array of Hash containing info about updated port.
    # @see #port_status_updated Each Hash instance included in the array is equivalent to port_status_updated argument Hash.
    # @return [void]
    #
    handler :all_port_status

    #
    # @!method all_link_status( link_stat )
    # get_all_link_status reply event handler,
    # when handler block was omitted on get_all_link_status call.
    # @abstract
    #
    # @param [Array<Hash>] link_stat
    #   Array of Hash containing info about updated link.
    # @see #link_status_updated Each Hash instance included in the array is equivalent to link_status_updated argument Hash.
    # @return [void]
    #
    handler :all_link_status

    # @!endgroup


    #
    # @!method start
    # Initialize topology as controller start.
    # This method will automatically subscribe to topology if topology handler was defined.
    #
    # This method will be implicitly called inside Controller#run! between init_trema() and start_trema() calls if not overridden by user.
    # @note Be sure to initialize and subscribe topology if overriding this method.
    #
    # @example
    #  class YourController < Controller
    #    include Topology
    #    def start
    #      init_libtopology "topology"
    #      send_subscribe_topology if topology_handler_implemented?
    #      send_enable_topology_discovery if respond_to?( :link_status_updated )
    #      # your application's pre-start_trema() call initialization here.
    #    end
    #  end
    #
    def start
      #  specify the name of topology service name
      init_libtopology "topology"
      send_subscribe_topology if topology_handler_implemented?
      send_enable_topology_discovery if respond_to?( :link_status_updated )
    end


    #
    # @!method shutdown!
    # Finalize topology before stopping trema.
    #
    def shutdown!
      finalize_libtopology
      super()
    end


    #######################
    protected
    #######################


    # @!group Topology update event subscription control


    #
    # @return true if subscribed to topology update notification.
    #
    def subscribed?
      return @is_subscribed
    end


    #
    # @!method subscribe_topology_reply(  )
    #
    # @abstract send_subscribe_topology reply event handler. Override this to implement a custom handler.
    #
    # Will be called as a reply to send_subscribe_topology, if defined.
    # @return [void]
    #
    handler :subscribe_topology_reply


    #
    # @!method unsubscribe_topology_reply(  )
    #
    # @abstract send_unsubscribe_topology reply event handler. Override this to implement a custom handler.
    #
    # Will be called as a reply to send_unsubscribe_topology, if defined.
    # @return [void]
    #
    handler :unsubscribe_topology_reply


    # @!group Discovery control

    #
    # @!method enable_topology_discovery_reply(  )
    #
    # @abstract send_enable_topology_discovery reply event handler. Override this to implement a custom handler.
    #
    # Will be called as a reply to send_enable_topology_discovery, if defined.
    # @return [void]
    #
    handler :enable_topology_discovery_reply


    #
    # @!method disable_topology_discovery_reply(  )
    #
    # @abstract send_disable_topology_discovery reply event handler. Override this to implement a custom handler.
    #
    # Will be called as a reply to send_disable_topology_discovery, if defined.
    # @return [void]
    #
    handler :disable_topology_discovery_reply

    # @!endgroup


    #
    # @return true if there exist a definition for topology update event handler
    #
    def topology_handler_implemented?
      return respond_to?( :switch_status_up ) | respond_to?( :switch_status_down ) |
             respond_to?( :switch_status_updated ) |
             respond_to?( :port_status_updated ) |
             respond_to?( :link_status_updated )
    end


    def _switch_status_updated sw
      switch_status_up sw[:dpid] if respond_to?( :switch_status_up ) and sw[:up]
      switch_status_updated sw if respond_to?( :switch_status_updated )
      switch_status_down sw[:dpid] if respond_to?( :switch_status_down ) and not sw[:up]
    end


    def _port_status_updated port
      port_status_updated port if respond_to?( :port_status_updated )
    end


    def _link_status_updated link
      link_status_updated link if respond_to?( :link_status_updated )
    end
  end
end
