#
# Trema executables.
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


require "trema/path"


class Trema::Executables
  def self.tremashark
    File.join Trema.objects, "tremashark/tremashark"
  end


  def self.packet_capture
    File.join Trema.objects, "tremashark/packet_capture"
  end


  def self.syslog_relay
    File.join Trema.objects, "tremashark/syslog_relay"
  end


  def self.stdin_relay
    File.join Trema.objects, "tremashark/stdin_relay"
  end


  def self.switch_manager
    File.join Trema.objects, "switch_manager/switch_manager"
  end


  def self.switch
    File.join Trema.objects, "switch_manager/switch"
  end


  def self.packetin_filter
    File.join Trema.objects, "packetin_filter/packetin_filter"
  end


  def self.phost
    File.join Trema.objects, "phost/phost"
  end


  def self.cli
    File.join Trema.objects, "phost/cli"
  end


  def self.ovs_openflowd
    File.join Trema.objects, "openvswitch/bin/ovs-openflowd"
  end


  def self.ovs_ofctl
    File.join Trema.objects, "openvswitch/bin/ovs-ofctl"
  end


  def self.compiled?
    m = singleton_methods - [ "const_missing", "to_yaml", "yaml_tag_subclasses?", "compiled?" ]
    m.inject( true ) do | result, each |
      result &&= FileTest.executable?( eval each )
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
