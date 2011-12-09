#
# Common commands supported by both trema command and shell.
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


require "trema/app"
require "trema/dsl/app"
require "trema/dsl/link"
require "trema/dsl/switch"
require "trema/dsl/vhost"
require "trema/dsl/vswitch"
require "trema/host"
require "trema/link"
require "trema/open-vswitch"
require "trema/openflow-switch"
require "trema/packetin-filter"
require "trema/switch-manager"
require "trema/tremashark"


module Trema::CommonCommands
  def use_tremashark
    @context.tremashark = Trema::Tremashark.new
  end


  def port number
    @context.port = number
  end


  def link peer0, peer1
    stanza = Trema::DSL::Link.new( peer0, peer1 )
    Trema::Link.new( stanza )
  end


  def switch name = nil, &block
    stanza = Trema::DSL::Switch.new( name )
    stanza.instance_eval( &block )
    Trema::OpenflowSwitch.new( stanza )
  end


  def vswitch name = nil, &block
    stanza = Trema::DSL::Vswitch.new( name )
    stanza.instance_eval( &block )
    Trema::OpenVswitch.new stanza, @context.port
  end


  def vhost name = nil, &block
    stanza = Trema::DSL::Vhost.new( name )
    stanza.instance_eval( &block ) if block
    Trema::Host.new( stanza )
  end


  def filter rule
    Trema::PacketinFilter.new( rule )
  end


  def event rule
    Trema::SwitchManager.new( rule, @context.port )
  end


  def app name = nil, &block
    stanza = Trema::DSL::App.new( name )
    stanza.instance_eval( &block )
    Trema::App.new( stanza )
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
