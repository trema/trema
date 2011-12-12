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


require "trema/daemon"
require "trema/executables"
require "trema/ofctl"
require "trema/openflow-switch"
require "trema/path"


module Trema
  #
  # Open vSwitch support (http://openvswitch.org)
  #
  class OpenVswitch < OpenflowSwitch
    include Trema::Daemon


    log_file { | vswitch | "openflowd.#{ vswitch.name }.log" }
    command { | vswitch | "sudo #{ Executables.ovs_openflowd } #{ vswitch.__send__ :options }" }


    #
    # Creates a new Open vSwitch from {DSL::Vswitch}
    #
    # @example
    #   switch = Trema::OpenVswitch.new( stanza, 6633 )
    #
    # @return [OpenVswitch]
    #
    def initialize stanza, port
      super stanza
      @port = port
      @interfaces = []
    end


    #
    # Add a network interface used for a virtual port
    #
    # @example
    #   switch.add_interface "trema3-0"
    #
    # @return [undefined]
    #
    def add_interface interface
      @interfaces << interface
      restart!
    end


    #
    # Returns the network device name associated with the datapath's
    # local port
    #
    # @example
    #   switch.network_device  #=> "vsw_0xabc"
    #
    # @return [String]
    #
    def network_device
      "vsw_#{ @stanza.get :dpid_short }"
    end


    #
    # Returns flow entries
    #
    # @example
    #   switch.flows  #=> [ flow0, flow1, ... ]
    #
    # @return [Array]
    #
    def flows
      Ofctl.new.users_flows( self )
    end


    ################################################################################
    private
    ################################################################################


    def options
      default_options.join( " " ) + " netdev@#{ network_device } tcp:#{ ip }:#{ @port }"
    end


    def default_options
      [
       "--detach",
       "--out-of-band",
       "--fail=closed",
       "--inactivity-probe=180",
       "--rate-limit=40000",
       "--burst-limit=20000",
       "--pidfile=#{ pid_file }",
       "--verbose=ANY:file:dbg",
       "--verbose=ANY:console:err",
       "--log-file=#{ log_file }",
       "--datapath-id=#{ dpid_long }",
       "--unixctl=#{ unixctl }",
      ] + ports_option
    end


    def ip
      @stanza.get :ip
    end


    def ports_option
      @interfaces.empty? ? [] : [ "--ports=#{ @interfaces.join( "," ) }" ]
    end


    def unixctl
      File.join Trema.tmp, "ovs-openflowd.#{ name }.ctl"
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
