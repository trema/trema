# encoding: utf-8
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

require File.join(File.dirname(__FILE__), '..', '..', 'spec_helper')
require 'trema/dsl/syntax'
require 'trema/packetin-filter'

describe Trema::DSL::Syntax do
  before(:each) do
    @context = double('context', :port => 6653, :dump_to => nil)
    @syntax = Trema::DSL::Syntax.new(@context)
  end

  it "should recognize 'port' directive" do
    expect(@context).to receive(:port=).with(1234).once

    @syntax.instance_eval do
      port 1234
    end
  end

  it "should recognize 'link' directive" do
    allow(@context).to receive(:links).and_return([double('link')])
    expect(Trema::Link).to receive(:add).with(an_instance_of(Trema::Link)).once

    @syntax.instance_eval do
      link 'PEER0', 'PEER1'
    end
  end

  it "should recognize 'switch' directive" do
    expect(Trema::OpenflowSwitch).to receive(:add).with(an_instance_of(HardwareSwitch)).once

    @syntax.instance_eval do
      switch { dpid '0xabc' }
    end
  end

  it "should recognize 'vswitch' directive" do
    expect(Trema::OpenflowSwitch).to receive(:add).with(an_instance_of(OpenVswitch)).once

    @syntax.instance_eval do
      vswitch { dpid '0xabc' }
    end
  end

  it "should recognize 'vhost' directive" do
    expect(Trema::Host).to receive(:add).with(an_instance_of(Trema::Host)).once

    @syntax.instance_eval do
      vhost {}
    end
  end

  it "should recognize 'filter' directive" do
    expect(Trema::PacketinFilter).to receive(:add).with(an_instance_of(Trema::PacketinFilter)).once

    @syntax.instance_eval do
      filter :lldp => 'LLDP RULE', :packet_in => 'PACKET-IN RULE'
    end
  end

  it "should recognize 'event' directive" do
    expect(Trema::SwitchManager).to receive(:add).with(an_instance_of(Trema::SwitchManager)).once

    @syntax.instance_eval do
      event 'RULE'
    end
  end

  it "should recognize 'run' directive" do
    expect(Trema::App).to receive(:add).with(an_instance_of(Trema::App)).once

    @syntax.instance_eval do
      run('My App') {}
    end
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
