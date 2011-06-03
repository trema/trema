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


$LOAD_PATH << File.join( File.dirname( __FILE__ ), "/../ruby" )


require "rubygems"

require "rspec"
require "trema/dsl/context"
require "trema/ofctl"
require "trema/shell-commands"
require "trema/util"


include Trema::Util


def drop_packets_from_unknown_hosts switch
  ofctl = Trema::Ofctl.new
  ofctl.add_flow switch, :priority => 0, :actions => "drop"
  @context.hosts.each do | name, each |
    ofctl.add_flow switch, :dl_type => "0x0800", :nw_src => each.ip, :priority => 1, :actions => "controller"
  end
end


def trema_run controller_class, &block
  cleanup_current_session

  @context = Trema::DSL::Parser.new.eval &block
  
  controller = controller_class.new
  Trema::App.add controller
  @context.dump_to Trema::DSL::Parser::CURRENT_CONTEXT

  app_name = controller.name
  rule = { :port_status => app_name, :packet_in => app_name, :state_notify => app_name }
  SwitchManager.new( rule, @context.port ).run!
  
  @context.links.each do | name, each |
    each.add!
  end
  @context.hosts.each do | name, each |
    each.run!
  end
  @context.switches.each do | name, each |
    each.run!
    loop do
      break if each.ready?
      sleep 0.1
    end
    drop_packets_from_unknown_hosts each
  end
  @context.links.each do | name, each |
    each.up!
  end
  @context.hosts.each do | name, each |
    each.add_arp_entry @context.hosts.values - [ each ]
  end

  @th_controller = Thread.start do
    controller.run!
  end
  sleep 2  # FIXME: wait until controller.up?
end


def trema_kill
  cleanup_current_session
  @th_controller.join
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
