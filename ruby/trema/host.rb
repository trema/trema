#
# The controller class of phost.
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


class Host
  attr_reader :name
  attr_reader :ip
  attr_reader :mac
  attr_accessor :interface


  def initialize stanza
    @name = stanza[ :name ]
    @ip = stanza[ :ip ]
    @netmask = stanza[ :netmask ]
    @mac = stanza[ :mac ]
  end


  def add_arp_entry hosts
    check_interface

    hosts.each do | each |
      sh "sudo #{ Trema::Executables.cli } -i #{ @interface } add_arp_entry --ip_addr #{ each.ip } --mac_addr #{ each.mac }"
    end
  end


  def run
    check_interface

    sh "sudo #{ Trema::Executables.phost } -i #{ @interface } -D"
    sleep 1
    sh "sudo #{ Trema::Executables.cli } -i #{ @interface } set_host_addr --ip_addr #{ @ip } --ip_mask #{ @netmask } --mac_addr #{ @mac }"
    sh "sudo #{ Trema::Executables.cli } -i #{ @interface } enable_promisc"
  end


  ################################################################################
  private
  ################################################################################


  def check_interface
    raise "The link(s) for vhost '#{ @name }' is not defined." if @interface.nil?
  end
end



### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
