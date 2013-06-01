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


require "trema/daemon"
require "trema/executables"
require "trema/network-component"
require "trema/switch-daemon"


module Trema
  #
  # The controller class of switch_manager
  #
  class SwitchManager < NetworkComponent
    include Trema::Daemon


    singleton_daemon
    command { | sm | sm.__send__ :command }


    #
    # Event forwarding rule
    #
    # @example
    #   switch_manager.rule => { :port_status => "topology manager", :packet_in => "controller", :state_notify => "topology manager" }
    #
    # @return [Hash]
    #
    attr_accessor :rule


    #
    # Do not cleanup the flow table of switches on startup
    #
    # @example
    #   switch_manager.no_flow_cleanup = true
    #
    # @return [Bool]
    #
    attr_accessor :no_flow_cleanup


    #
    # Creates a switch manager controller
    #
    # @example
    #   rule = { :port_status => "topology manager", :packet_in => "controller", :state_notify => "topology manager", :vendor => "controller" }
    #   switch_manager = Trema::SwitchManager.new( rule )
    #
    # @return [SwitchManager]
    #
    def initialize rule, port = nil
      @rule = rule
      @port = port
      @no_flow_cleanup = false
      SwitchManager.add self
    end


    #
    # Returns the name of switch manager
    #
    # @example
    #   switch_maanger.name => "switch manager"
    #
    # @return [String]
    #
    def name
      "switch manager"
    end


    ############################################################################
    private
    ############################################################################


    def command
      "#{ Executables.switch_manager } #{ options.join " " } -- #{ switch_options.join " " }"
    end


    def options
      opts = [ "--daemonize" ]
      opts << "--port=#{ @port }" if @port
      opts
    end


    def switch_options
      opts = SwitchDaemon.new( @rule ).options
      opts << "--no-flow-cleanup" if @no_flow_cleanup
      opts
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
