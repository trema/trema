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
require "trema/shell-commands"


def trema_run controller_class, &block
  system "./trema kill"

  @context = Trema::DSL::Parser.new.eval &block
  
  controller = controller_class.new
  Trema::App.add controller

  app_name = controller.name
  rule = { :port_status => app_name, :packet_in => app_name, :state_notify => app_name }
  SwitchManager.new( rule, @context.port ).run!
  
  @context.links.each do | name, link |
    link.add!
  end
  @context.hosts.each do | name, host |
    host.run!
  end
  @context.switches.each do | name, switch |
    switch.run_rspec!
  end
  @context.links.each do | name, link |
    link.up!
  end
  @context.hosts.each do | name, host |
    host.add_arp_entry @context.hosts.values - [ host ]
  end

  @controller_thread = Thread.start do
    controller.run!
  end
  sleep 3  # FIXME
end


def trema_kill
  if @controller_thread
    Trema::App.each do | each |
      each.shutdown!
    end
    @controller_thread.join
  end
  return if @context.nil?

  @context.links.each do | name, link |
    link.delete!
  end
  @context.switches.each do | name, switch |
    switch.shutdown!
  end
  @context.hosts.each do | name, host |
    host.shutdown!
  end

  Dir.glob( File.join Trema.tmp, "*.pid" ).each do | each |
    Trema::Process.read( each ).kill!
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
