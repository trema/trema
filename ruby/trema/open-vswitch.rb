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


require "trema/default_openflow_channel_port"
require "trema/daemon"
require "trema/executables"
require "trema/hardware-switch"
require "trema/ofctl"
require "trema/path"


module Trema
  #
  # Open vSwitch support (http://openvswitch.org)
  #
  class OpenVswitch < HardwareSwitch
    include Trema::Daemon


    log_file { | vswitch | "openflowd.#{ vswitch.name }.log" }
    command { | vswitch | vswitch.__send__ :command }


    #
    # Creates a new Open vSwitch from {DSL::Vswitch}
    #
    # @example
    #   vswitch = Trema::OpenVswitch.new( stanza )
    #
    # @return [OpenVswitch]
    #
    def initialize stanza, port = DEFAULT_OPENFLOW_CHANNEL_PORT
      super stanza
      @port = port
      @interfaces = []
    end


    #
    # Add a network interface used for a virtual port
    #
    # @example
    #   vswitch << "trema3-0"
    #
    # @return [Array]
    #
    def << interface
      @interfaces << interface
      restart!
      @interfaces
    end


    #
    # Returns the network device name associated with the datapath's
    # local port
    #
    # @example
    #   vswitch.network_device  #=> "vsw_0xabc"
    #
    # @return [String]
    #
    def network_device
      "vsw_#{ @stanza.fetch :dpid_short }"
    end


    #
    # Returns flow entries
    #
    # @example
    #   vswitch.flows  #=> [ flow0, flow1, ... ]
    #
    # @return [Array]
    #
    def flows
      Ofctl.new.users_flows( self )
    end


    def bring_port_up port_number
      Ofctl.new.bring_port_up self, port_number
    end


    def bring_port_down port_number
      Ofctl.new.bring_port_down self, port_number
    end


    ############################################################################
    private
    ############################################################################


    def command
      "sudo #{ Executables.ovs_openflowd } #{ options }"
    end


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
      @stanza.fetch :ip
    end


    def ports_option
      @interfaces.empty? ? [] : [ "--ports=#{ @interfaces.join( "," ) }" ]
    end


    def unixctl
      File.join Trema.sock, "ovs-openflowd.#{ name }.ctl"
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
