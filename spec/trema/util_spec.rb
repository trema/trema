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

require File.join(File.dirname(__FILE__), '..', 'spec_helper')
require 'trema/dsl/configuration'
require 'trema/dsl/runner'
require 'trema/util'

include Trema::Util

describe Trema::Util do
  it 'should assert that trema is built' do
    allow(Trema::Executables).to receive(:compiled?).and_return(false)
    expect($stderr).to receive(:puts).with(/^ERROR/)
    expect { assert_trema_is_built }.to raise_error(SystemExit)
  end

  it 'should execute and check the results of a command' do
    expect { sh 'NO SUCH COMMAND' }.to raise_error("Command 'NO SUCH COMMAND' failed!")
  end

  it 'should cleanup current session' do
    apps = { 'app 1' => double('app 1'), 'app 2' => double('app 2'), 'app 3' => double('app 3') }
    apps.each do |_name, app|
      expect(app).to receive(:shutdown!)
    end

    switches = { 'switch 1' => double('switch 1'), 'switch 2' => double('switch 2'), 'switch 3' => double('switch 3') }
    switches.each do |_name, switch|
      expect(switch).to receive(:shutdown!)
    end

    hosts = { 'host 1' => double('host 1'), 'host 2' => double('host 2'), 'host 3' => double('host 3') }
    hosts.each do |_name, host|
      expect(host).to receive(:shutdown!)
    end

    links = { 'link 1' => double('link 1'), 'link 2' => double('link 2'), 'link 3' => double('link 3') }
    links.each do |_name, link|
      expect(link).to receive(:delete!)
    end

    last_session = double('last session')
    allow(last_session).to receive(:apps).and_return(apps)
    allow(last_session).to receive(:switches).and_return(switches)
    allow(last_session).to receive(:hosts).and_return(hosts)
    allow(last_session).to receive(:links).and_return(links)
    allow(last_session).to receive(:netnss).and_return({})
    allow(Trema::DSL::Context).to receive(:load_current).and_return(last_session)

    pid_files = [double('PID file #0'), double('PID file #1'), double('PID file #2')]
    allow(Dir).to receive(:glob).and_return(pid_files)

    process = double('process')
    expect(process).to receive(:kill!).exactly(3).times
    expect(Trema::Process).to receive(:read).with(pid_files[0]).once.ordered.and_return(process)
    expect(Trema::Process).to receive(:read).with(pid_files[1]).once.ordered.and_return(process)
    expect(Trema::Process).to receive(:read).with(pid_files[2]).once.ordered.and_return(process)

    cleanup_current_session
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
