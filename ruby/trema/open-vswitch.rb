#
# The controller class of Open vSwitch.
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


require "fileutils"
require "trema/executables"
require "trema/flow"
require "trema/ofctl"
require "trema/openflow-switch"
require "trema/path"
require "trema/process"
require "trema/switch"


module Trema
  #
  # Open vSwitch http://openvswitch.org/
  #
  class OpenVswitch < OpenflowSwitch
    def initialize stanza, port
      super stanza
      @port = port
      @interfaces = []
      @ofctl = Trema::Ofctl.new
    end


    def add_interface name
      @interfaces << name
    end


    def datapath
      "vsw_#{ @name }"
    end


    def run!
      FileUtils.rm_f log_file
      sh "sudo #{ Executables.ovs_openflowd } #{ options }"
    end


    def run_rspec!
      run!
      drop_packets_from_unknown_hosts
      @log = read_log_all
    end


    def shutdown!
      if @log
        examine_log
        @log.close
      end
      Trema::Process.read( pid_file, @name ).kill!
    end


    def dump_flows
      puts @ofctl.dump_flows( self )
    end
    

    def flows
      @ofctl.flows( self ).select do | each |
        not each.rspec_flow?
      end
    end
    

    ################################################################################
    private
    ################################################################################


    def drop_packets_from_unknown_hosts
      @ofctl.add_flow self, :priority => 0, :actions => "drop"
      Trema::Host.each do | each |
        @ofctl.add_flow self, :dl_type => "0x0800", :nw_src => each.ip, :priority => 1, :actions => "controller"
      end
    end


    def read_log_all
      log = File.open( log_file, "r" )
      log.read
      log
    end
    

    def flow_mod_add line
    end
    

    def examine_log
      while @log.gets
        if /received: flow_mod \(xid=.+\):.* ADD:/=~ $_
          flow_mod_add $_
        end
      end
    end
    

    def ip
      @stanza[ :ip ]
    end


    def options
      default_options.join( " " ) + " netdev@#{ datapath } tcp:#{ ip }:#{ @port }"
    end


    def default_options
      [
       "--detach",
       "--out-of-band",
       "--no-resolv-conf",
       "--fail=closed",
       "--inactivity-probe=180",
       "--rate-limit=40000",
       "--burst-limit=20000",
       "--pidfile=#{ pid_file }",
       "--verbose=ANY:file:dbg",
       "--verbose=ANY:console:err",
       "--log-file=#{ log_file }",
       "--datapath-id=#{ dpid_long }",
      ] + ports_option
    end


    def pid_file
      File.join Trema.tmp, "openflowd.#{ @name }.pid"
    end
    

    def log_file
      "#{ Trema.tmp }/log/openflowd.#{ @name }.log"
    end


    def ports_option
      @interfaces.empty? ? [] : [ "--ports=#{ @interfaces.join( "," ) }" ]
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
