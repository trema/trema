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


require File.join( File.dirname( __FILE__ ), "..", "spec_helper" )
require "trema/switch-daemon"


module Trema
  describe SwitchDaemon do
    it "should be initialized with proper options" do
      rule = {
        :port_status => "topology",
        :packet_in => "controller",
        :state_notify => "topology",
        :vendor => "controller"
      }
      switch_daemon = SwitchDaemon.new( rule )

      expect( switch_daemon.options ).to include( "port_status::topology" )
      expect( switch_daemon.options ).to include( "packet_in::controller" )
      expect( switch_daemon.options ).to include( "state_notify::topology" )
      expect( switch_daemon.options ).to include( "vendor::controller" )
    end


    it "should be initialized with options which have multiple controllers" do
      rule = {
        :port_status => "topology0,topology1",
        :packet_in => "controller0,controller1",
        :state_notify => "topology0,topology1",
        :vendor => "controller0,controller1"
      }
      switch_daemon = SwitchDaemon.new( rule )

      expect( switch_daemon.options ).to include( "port_status::topology0" )
      expect( switch_daemon.options ).to include( "port_status::topology1" )
      expect( switch_daemon.options ).to include( "packet_in::controller0" )
      expect( switch_daemon.options ).to include( "packet_in::controller1" )
      expect( switch_daemon.options ).to include( "state_notify::topology0" )
      expect( switch_daemon.options ).to include( "state_notify::topology1" )
      expect( switch_daemon.options ).to include( "vendor::controller0" )
      expect( switch_daemon.options ).to include( "vendor::controller1" )
    end


    it "should be initialized without vendor options" do
      rule = {
        :port_status => "topology",
        :packet_in => "controller",
        :state_notify => "topology"
      }
      switch_daemon = SwitchDaemon.new( rule )

      expect( switch_daemon.options ).to include( "port_status::topology" )
      expect( switch_daemon.options ).to include( "packet_in::controller" )
      expect( switch_daemon.options ).to include( "state_notify::topology" )
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
