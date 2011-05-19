#
# The controller class of phost cli.
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


class Cli
  def initialize host
    @host = host
  end
  
  
  def send_packets dest, options = {}
    if options[ :duration ] and options[ :n_pkts ]
      raise "--duration and --n_pkts are exclusive."
    end

    sh( "#{ Trema::Executables.cli } -i #{ @host.interface } send_packets " +
        "--ip_src #{ @host.ip } --ip_dst #{ dest.ip } " + 
        send_packets_options( options ) )
  end


  def show_tx_stats
    show_stats :tx
  end


  def show_rx_stats
    show_stats :rx
  end


  def reset_stats
    sh "sudo #{ Trema::Executables.cli } -i #{ @host.interface } reset_stats"
  end


  def add_arp_entry other
    sh "sudo #{ Trema::Executables.cli } -i #{ @host.interface } add_arp_entry --ip_addr #{ other.ip } --mac_addr #{ other.mac }"
  end
  

  def set_ip_and_mac_address
    sh "sudo #{ Trema::Executables.cli } -i #{ @host.interface } set_host_addr --ip_addr #{ @host.ip } --ip_mask #{ @host.netmask } --mac_addr #{ @host.mac }"
  end


  def enable_promisc
    sh "sudo #{ Trema::Executables.cli } -i #{ @host.interface } enable_promisc"
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


  def show_stats type
    sh "sudo #{ Trema::Executables.cli } -i #{ @host.interface } show_stats --#{ type }"
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
