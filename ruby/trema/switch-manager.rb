#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
#
# Copyright (C) 2008-2011 NEC Corporation
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
require "trema/switch-daemon"


module Trema
  #
  # The controller class of switch_manager
  #
  class SwitchManager < NetworkComponent
    #
    # Event forwarding rule
    #
    # @example
    #   switch_manager.rule => { :port_status => "topology manager", :packet_in => "controller", :state_notify => "topology manager" }
    #
    # @return [Hash]
    #
    # @api public
    #
    attr_accessor :rule


    #
    # Creates a switch manager controller
    #
    # @example
    #   rule = { :port_status => "topology manager", :packet_in => "controller", :state_notify => "topology manager", :vendor => "controller" }
    #   switch_manager = Trema::SwitchManager.new( rule )
    #
    # @return [SwitchManager]
    #
    # @api public
    #
    def initialize rule, port = nil
      @rule = rule
      @port = port
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
    # @api public
    #
    def name
      "switch manager"
    end


    #
    # Runs an switch manager process
    #
    # @example
    #   switch_manager.run!
    #
    # @return [undefined]
    #
    # @api public
    #
    def run! switch_options = []
      sh "#{ Executables.switch_manager } #{ options.join " " } -- #{ ( switch_options + default_switch_options ).join " " }"
    end


    ################################################################################
    private
    ################################################################################


    #
    # Returns the command line options
    #
    # @return [Array]
    #
    # @api private
    #
    def options
      opts = [ "--daemonize" ]
      opts << "--port=#{ @port }" if @port
      opts
    end


    #
    # Returns the command line options of switch daemon ({SwitchDaemon})
    #
    # @return [Array]
    #
    # @api private
    #
    def default_switch_options
      SwitchDaemon.new( @rule ).options
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
