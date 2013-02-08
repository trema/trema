#
# The controller class of phost cli.
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


module Trema
  class Stats
    class Packet
    end


    attr_reader :ip_dst
    attr_reader :tp_dst
    attr_reader :ip_src
    attr_reader :tp_src
    attr_reader :n_pkts
    attr_reader :n_octets


    def initialize ip_dst, tp_dst, ip_src, tp_src, n_pkts, n_octets
      @ip_dst = ip_dst
      @tp_dst = tp_dst.to_i
      @ip_src = ip_src
      @tp_src = tp_src.to_i
      @n_pkts = n_pkts.to_i
      @n_octets = n_octets.to_i
    end


    def packets
      list = []
      @n_pkts.times do
        list << Packet.new
      end
      list
    end
  end
end


module Trema
  class Cli
    def initialize host
      @host = host
    end


    def send_packets dest, options = {}
      if options[ :duration ] and options[ :n_pkts ]
        raise "--duration and --n_pkts are exclusive."
      end

      sh( "#{ Executables.cli } -i #{ @host.interface } send_packets " +
          "--ip_src #{ @host.ip } --ip_dst #{ dest.ip } " +
          send_packets_options( options ) )
    end


    def show_tx_stats
      puts stats( :tx )
    end


    def show_rx_stats
      puts stats( :rx )
    end


    def tx_stats
      stat = stats( :tx ).split( "\n" )[ 1 ]
      if stat
        Trema::Stats.new *stat.split( "," )
      else
        nil
      end
    end


    def rx_stats
      stat = stats( :rx ).split( "\n" )[ 1 ]
      if stat
        Trema::Stats.new *stat.split( "," )
      else
        Trema::Stats.new nil, nil, nil, nil, 0, 0
      end
    end


    def reset_stats
      sh "sudo #{ Executables.cli } -i #{ @host.interface } reset_stats --tx"
      sh "sudo #{ Executables.cli } -i #{ @host.interface } reset_stats --rx"
    end


    def add_arp_entry other
      sh "sudo #{ Executables.cli } -i #{ @host.interface } add_arp_entry --ip_addr #{ other.ip } --mac_addr #{ other.mac }"
    end


    def set_ip_and_mac_address
      sh "sudo #{ Executables.cli } -i #{ @host.interface } set_host_addr --ip_addr #{ @host.ip } --ip_mask #{ @host.netmask } --mac_addr #{ @host.mac }"
    end


    def enable_promisc
      sh "sudo #{ Executables.cli } -i #{ @host.interface } enable_promisc"
    end


    ################################################################################
    private
    ################################################################################


    def send_packets_options options
      [
       tp_src( options[ :tp_src ] || default_tp_src ),
       tp_dst( options[ :tp_dst ] || default_tp_dst ),
       pps( options[ :pps ] || default_pps ),
       options[ :n_pkts ] ? nil : duration( options[ :duration ] || default_duration ),
       length( options[ :length ] || default_length ),
       n_pkts( options[ :n_pkts ] ),
       inc_ip_src( options[ :inc_ip_src ] ),
       inc_ip_dst( options[ :inc_ip_dst ] ),
       inc_tp_src( options[ :inc_tp_src ] ),
       inc_tp_dst( options[ :inc_tp_dst ] ),
       inc_payload( options[ :inc_payload ] ),
      ].compact.join( " " )
    end


    def tp_src value
      "--tp_src #{ value }"
    end


    def default_tp_src
      1
    end


    def tp_dst value
      "--tp_dst #{ value }"
    end


    def default_tp_dst
      1
    end


    def pps value
      "--pps #{ value }"
    end


    def default_pps
      1
    end


    def duration value
      "--duration #{ value }"
    end


    def default_duration
      1
    end


    def length value
      "--length #{ value }"
    end


    def default_length
      22
    end


    def inc_ip_src value
      return nil if value.nil?
      if value == true
        "--inc_ip_src"
      else
        "--inc_ip_src=#{ value }"
      end
    end


    def inc_ip_dst value
      return nil if value.nil?
      if value == true
        "--inc_ip_dst"
      else
        "--inc_ip_dst=#{ value }"
      end
    end


    def inc_tp_src value
      return nil if value.nil?
      if value == true
        "--inc_tp_src"
      else
        "--inc_tp_src=#{ value }"
      end
    end


    def inc_tp_dst value
      return nil if value.nil?
      if value == true
        "--inc_tp_dst"
      else
        "--inc_tp_dst=#{ value }"
      end
    end


    def inc_payload value
      return nil if value.nil?
      if value == true
        "--inc_payload"
      else
        "--inc_payload=#{ value }"
      end
    end


    def n_pkts value
      return nil if value.nil?
      "--n_pkts=#{ value }"
    end


    def stats type
      `sudo #{ Executables.cli } -i #{ @host.interface } show_stats --#{ type }`
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
