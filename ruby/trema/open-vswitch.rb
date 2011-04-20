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
require "trema/openflow-switch"
require "trema/path"


class OpenVswitch < OpenflowSwitch
  def initialize stanza, port
    super stanza
    @port = port
    @interfaces = []
  end


  def add_interface name
    @interfaces << name
  end


  def datapath
    "vsw_#{ @name }"
  end


  def run
    FileUtils.rm_f log_file
    sh "sudo #{ Trema::Executables.ovs_openflowd } #{ options } netdev@#{ datapath } tcp:#{ ip }:#{ @port }"
  end


  ################################################################################
  private
  ################################################################################


  def ip
    @stanza[ :ip ]
  end


  def options
    ( default_options + ports_option ).join( " " )
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
      "--pidfile=#{ Trema.home }/tmp/openflowd.#{ @name }.pid",
      "--verbose=ANY:file:dbg",
      "--verbose=ANY:console:err",
      "--log-file=#{ log_file }",
      "--datapath-id=#{ dpid_long }",
    ]
  end


  def log_file
    "#{ Trema.home }/tmp/log/openflowd.#{ @name }.log"
  end


  def ports_option
    @interfaces.empty? ? [] : [ "--ports=#{ @interfaces.join( "," ) }" ]
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
