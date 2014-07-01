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
require 'trema/dsl/runner'
require 'trema/ordered-hash'

module Trema
  module DSL
    describe Runner do
      before :each do
        allow(::Process).to receive(:fork).and_yield
        allow(::Process).to receive(:waitpid)
        @switch_manager = double('switch manager', :run! => nil)
        allow(SwitchManager).to receive(:new).and_return(@switch_manager)
      end

      context 'when running' do
        it 'should run switch_manager' do
          expect(@switch_manager).to receive(:run!).once

          context = double(
            'context',
            :port => 6653,
            :tremashark => nil,
            :switch_manager => nil,
            :packetin_filter => nil,
            :links => {},
            :hosts => {},
            :switches => {},
            :apps => {},
            :netnss => {}
          )

          Runner.new(context).run
        end

        it 'should run packetin_filter' do
          packetin_filter = double
          expect(packetin_filter).to receive(:run!).once

          context = double(
            'context',
            :tremashark => nil,
            :switch_manager => double('switch_manager', :run! => nil),
            :packetin_filter => packetin_filter,
            :links => {},
            :hosts => {},
            :switches => {},
            :apps => {},
            :netnss => {},
            :port => 6633
          )

          Runner.new(context).run
        end

        it 'should create links' do
          link0 = double('link0')
          expect(link0).to receive(:delete!).once
          expect(link0).to receive(:enable!).once

          link1 = double('link1')
          expect(link1).to receive(:delete!).once
          expect(link1).to receive(:enable!).once

          link2 = double('link2')
          expect(link2).to receive(:delete!).once
          expect(link2).to receive(:enable!).once

          context = double(
            'context',
            :tremashark => nil,
            :switch_manager => double('switch manager', :run! => nil),
            :packetin_filter => nil,
            :links => { 'link0' => link0, 'link1' => link1, 'link2' => link2 },
            :hosts => {},
            :switches => {},
            :apps => {},
            :netnss => {},
            :port => 6633
          )

          Runner.new(context).run
        end

        it 'should run vhosts' do
          pending

          host0 = double('host0')
          host1 = double('host1')
          host2 = double('host2')

          expect(host0).to receive(:run!).once
          expect(host0).to receive(:add_arp_entry).with { | arg |
            expect(arg.size).to eq(2)
            expect(arg).to include(host1)
            expect(arg).to include(host2)
          }

          expect(host1).to receive(:run!).once
          expect(host1).to receive(:add_arp_entry).with { | arg |
            expect(arg.size).to eq(2)
            expect(arg).to include(host0)
            expect(arg).to include(host2)
          }

          expect(host2).to receive(:run!).once
          expect(host2).to receive(:add_arp_entry).with { | arg |
            expect(arg.size).to eq(2)
            expect(arg).to include(host0)
            expect(arg).to include(host1)
          }

          context = double(
            'context',
            :tremashark => nil,
            :switch_manager => double('switch manager', :run! => nil),
            :packetin_filter => nil,
            :links => {},
            :hosts => { 'host0' => host0, 'host1' => host1, 'host2' => host2 },
            :switches => {},
            :apps => {},
            :netnss => {},
            :port => 6633
          )

          Runner.new(context).run
        end

        it 'should run switches' do
          switch0 = double('switch0')
          expect(switch0).to receive(:run!).once

          switch1 = double('switch1')
          expect(switch1).to receive(:run!).once

          switch2 = double('switch2')
          expect(switch2).to receive(:run!).once

          context = double(
            'context',
            :tremashark => nil,
            :switch_manager => double('switch manager', :run! => nil),
            :packetin_filter => nil,
            :links => {},
            :hosts => {},
            :switches => { 'switch0' => switch0, 'switch1' => switch1, 'switch 2' => switch2 },
            :apps => {},
            :netnss => {},
            :port => 6633
          )

          Runner.new(context).run
        end

        it 'should run apps' do
          apps = OrderedHash.new

          app0 = double('app0', :name => 'app0')
          expect(app0).to receive(:daemonize!).once.ordered
          apps[ 'app0'] = app0

          app1 = double('app1', :name => 'app1')
          expect(app1).to receive(:daemonize!).once.ordered
          apps[ 'app1'] = app1

          app2 = double('app2', :name => 'app2')
          expect(app2).to receive(:run!).once.ordered
          apps[ 'app2'] = app2

          context = double(
            'context',
            :tremashark => nil,
            :switch_manager => double('switch manager', :run! => nil, :rule => {}),
            :packetin_filter => nil,
            :links => {},
            :hosts => {},
            :switches => {},
            :netnss => {},
            :apps => apps
          )

          Runner.new(context).run
        end

        it 'should daemonize apps' do
          apps = OrderedHash.new

          app0 = double('app0')
          expect(app0).to receive(:daemonize!).once.ordered
          apps[ 'app0'] = app0

          app1 = double('app1')
          expect(app1).to receive(:daemonize!).once.ordered
          apps[ 'app1'] = app1

          app2 = double('app2', :name => 'App2')
          expect(app2).to receive(:daemonize!).once.ordered
          apps[ 'app2'] = app2

          context = double(
            'context',
            :tremashark => nil,
            :switch_manager => double('switch manager', :run! => nil, :rule => {}),
            :packetin_filter => nil,
            :links => {},
            :hosts => {},
            :switches => {},
            :netnss => {},
            :apps => apps
          )

          Runner.new(context).daemonize
        end
      end
    end
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
