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
require 'trema/process'

describe Trema::Process do
  before :each do
    @pid_file = double('PID file')
  end

  it 'should be instantiated from a PID file' do
    expect(IO).to receive(:read).with(@pid_file).and_return("1234\n")
    stat = double('stat', :uid => 0)
    allow(File).to receive(:stat).with(@pid_file).and_return(stat)

    Trema::Process.read(@pid_file)
  end

  it 'should be killed' do
    allow(IO).to receive(:read).with(@pid_file).and_return("1234\n")
    stat = double('stat', :uid => 1000)
    allow(File).to receive(:stat).with(@pid_file).and_return(stat)

    process = Trema::Process.read(@pid_file)
    allow(process).to receive(:`).and_return('ALIVE', '')
    expect(process).to receive(:sh).with('kill 1234 2>/dev/null')

    process.kill!
  end

  it 'should be killed with sudo' do
    allow(IO).to receive(:read).with(@pid_file).and_return("1234\n")
    stat = double('stat', :uid => 0)
    allow(File).to receive(:stat).with(@pid_file).and_return(stat)

    process = Trema::Process.read(@pid_file)
    allow(process).to receive(:`).and_return('ALIVE', '')
    expect(process).to receive(:sh).with('sudo kill 1234 2>/dev/null')

    process.kill!
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
