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


require "trema/cli"
require "trema/executables"


class Host
  attr_reader :name
  attr_reader :ip
  attr_reader :mac
  attr_reader :netmask
  attr_accessor :interface


  def initialize stanza
    @name = stanza[ :name ]
    @ip = stanza[ :ip ]
    @mac = stanza[ :mac ]    
    @netmask = stanza[ :netmask ]
    @promisc = stanza[ :promisc ]

    @cli = Cli.new
  end


  def add_arp_entry hosts
    hosts.each do | each |
      @cli.add_arp_entry self, each
    end
  end


  def run
    raise "The link(s) for vhost '#{ @name }' is not defined." if @interface.nil?

    sh "sudo #{ Trema::Executables.phost } -i #{ @interface } -D"
    wait_until_up
    @cli.set_host_addr self
    @cli.enable_promisc( self ) if @promisc
  end


  def send_packet options
    @cli.send_packets self, Trema::Vhost[ options[ :to ] ]
  end


  ################################################################################
  private
  ################################################################################


  def wait_until_up
    loop do
      break if FileTest.exists?( pid_file )
      sleep 0.1
    end
  end


  def pid_file
    File.join Trema.tmp, "phost.#{ @interface }.pid"
  end
end



### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
