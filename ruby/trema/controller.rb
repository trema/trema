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


require "trema/app"
require "trema/default-logger"
require "trema/monkey-patch/integer"
require "trema/monkey-patch/string"
require "trema/timers"


module Trema
  #
  # @abstract The base class of Trema controller. Subclass and override handlers to implement a custom OpenFlow controller.
  #
  class Controller < App
    include DefaultLogger
    include Timers


    #
    # @private Just a placeholder for YARD.
    #
    def self.handler name
      # Do nothing.
    end


    #
    # @!method switch_ready( datapath_id )
    #
    # @abstract Switch Ready event handler. Override this to implement a custom handler.
    #
    # @param [Integer] datapath_id
    #   the datapath ID of connected OpenFlow switch.
    #
    handler :switch_ready


    #
    # @!method switch_disconnected( datapath_id )
    #
    # @abstract Switch Disconnected event handler. Override this to implement a custom handler.
    #
    # @param [Integer] datapath_id
    #   the datapath ID of disconnected OpenFlow switch.
    #
    handler :switch_disconnected



    #
    # @!method list_switches_reply( datapath_ids )
    #
    # @abstract List Switches Reply message handler. Override this to implement a custom handler.
    #
    # @param [Array<Integer>] datapath_ids
    #   the datapath IDs of connected OpenFlow switches.
    #
    handler :list_switches_reply


    #
    # @!method packet_in( datapath_id, message )
    #
    # @abstract Packet In message handler. Override this to implement a custom handler.
    #
    # @param [Integer] datapath_id
    #   the datapath from which a message is sent.
    # @param [PacketIn] message
    #   the Packet In message.
    #
    handler :packet_in


    #
    # @!method flow_removed( datapath_id, message )
    #
    # @abstract Flow Removed message handler. Override this to implement a custom handler.
    #
    # @param [Integer] datapath_id
    #   the datapath from which a message is sent.
    # @param [FlowRemoved] message
    #   the Flow Removed message.
    #
    handler :flow_removed


    #
    # @!method port_status( datapath_id, message )
    #
    # @abstract Port Status message handler. Override this to implement a custom handler.
    #
    # @param [Integer] datapath_id
    #   the datapath from which a message is sent.
    # @param [PortStatus] message
    #   the Port Status message.
    #
    handler :port_status


    #
    # @!method openflow_error( datapath_id, message )
    #
    # @abstract OpenFlow Error message handler. Override this to implement a custom handler.
    #
    # @param [Integer] datapath_id
    #   the datapath from which a message is sent.
    # @param [OpenflowError] message
    #   the OpenFlow Error message.
    #
    handler :openflow_error


    #
    # @!method features_reply( datapath_id, message )
    #
    # @abstract Features Reply message handler. Override this to implement a custom handler.
    #
    # @param [Integer] datapath_id
    #   the datapath from which a message is sent.
    # @param [FeaturesReply] message
    #   the Features Reply message.
    #
    handler :features_reply


    #
    # @!method stats_reply( datapath_id, message )
    #
    # @abstract Stats Reply message handler. Override this to implement a custom handler.
    #
    # @param [Integer] datapath_id
    #   the datapath from which a message is sent.
    # @param [StatsReply] message
    #   the Stats Reply message.
    #
    handler :stats_reply


    #
    # @!method barrier_reply( datapath_id, message )
    #
    # @abstract Barrier Reply message handler. Override this to implement a custom handler.
    #
    # @param [Integer] datapath_id
    #   the datapath from which a message is sent.
    # @param [BarrierReply] message
    #   the Barrier Reply message.
    #
    handler :barrier_reply


    #
    # @!method get_config_reply( datapath_id, message )
    #
    # @abstract Get Config Reply message handler. Override this to implement a custom handler.
    #
    # @param [Integer] datapath_id
    #   the datapath from which a message is sent.
    # @param [GetConfigReply] message
    #   the Get Config Reply message.
    #
    handler :get_config_reply


    #
    # @!method queue_get_config_reply( datapath_id, message )
    #
    # @abstract Queue Get Config Reply message handler. Override this to implement a custom handler.
    #
    # @param [Integer] datapath_id
    #   the datapath from which a message is sent.
    # @param [QueueGetConfigReply] message
    #   the Queue Get Config Reply message.
    #
    handler :queue_get_config_reply


    #
    # @!method vendor( datapath_id, message )
    #
    # @abstract Vendor message handler. Override this to implement a custom handler.
    #
    # @param [Integer] datapath_id
    #   the datapath from which a message is sent.
    # @param [Vendor] message
    #   the Vendor message.
    #
    handler :vendor


    #
    # @private
    #
    def self.inherited subclass
      subclass.new
    end


    #
    # @private
    #
    def initialize
      App.add self
    end


    #
    # Run as a daemon.
    #
    def daemonize!
      fork do
        ::Process.setsid
        fork do
          STDIN.close
          STDOUT.reopen "/dev/null", "a"
          STDERR.reopen "/dev/null", "a"
          self.run!
        end
      end
    end


    #
    # Name of the controller.
    #
    # @return [String]
    #
    def name
      self.class.to_s.split( "::" ).last
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
