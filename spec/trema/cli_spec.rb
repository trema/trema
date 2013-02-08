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
require "trema/cli"


module Trema
  describe Cli do
    before :each do
      @source = mock( "source", :interface => "trema-0", :ip => "192.168.0.1" )
      @dest = mock( "dest", :interface => "trema-1", :ip => "192.168.0.2" )
      @cli = Cli.new( @source )
    end


    context "when sending packets" do
      it "should send packets (default)" do
        @cli.should_receive( :sh ).with( /cli -i trema-0 send_packets --ip_src 192.168.0.1 --ip_dst 192.168.0.2 --tp_src 1 --tp_dst 1 --pps 1 --duration 1 --length 22$/ )
        @cli.send_packets( @dest )
      end


      it "should send packets (inc_ip_src)" do
        @cli.should_receive( :sh ).with( /cli -i trema-0 send_packets --ip_src 192.168.0.1 --ip_dst 192.168.0.2 --tp_src 1 --tp_dst 1 --pps 1 --duration 1 --length 22 --inc_ip_src$/ )
        @cli.send_packets( @dest, :inc_ip_src => true )
      end


      it "should send packets (inc_ip_src = 1)" do
        @cli.should_receive( :sh ).with( /cli -i trema-0 send_packets --ip_src 192.168.0.1 --ip_dst 192.168.0.2 --tp_src 1 --tp_dst 1 --pps 1 --duration 1 --length 22 --inc_ip_src=1$/ )
        @cli.send_packets( @dest, :inc_ip_src => 1 )
      end


      it "should send packets (inc_ip_dst)" do
        @cli.should_receive( :sh ).with( /cli -i trema-0 send_packets --ip_src 192.168.0.1 --ip_dst 192.168.0.2 --tp_src 1 --tp_dst 1 --pps 1 --duration 1 --length 22 --inc_ip_dst$/ )
        @cli.send_packets( @dest, :inc_ip_dst => true )
      end


      it "should send packets (inc_ip_dst = 1)" do
        @cli.should_receive( :sh ).with( /cli -i trema-0 send_packets --ip_src 192.168.0.1 --ip_dst 192.168.0.2 --tp_src 1 --tp_dst 1 --pps 1 --duration 1 --length 22 --inc_ip_dst=1$/ )
        @cli.send_packets( @dest, :inc_ip_dst => 1 )
      end


      it "should send_packets (tp_src = 60000)" do
        @cli.should_receive( :sh ).with( /cli -i trema-0 send_packets --ip_src 192.168.0.1 --ip_dst 192.168.0.2 --tp_src 60000 --tp_dst 1 --pps 1 --duration 1 --length 22$/ )
        @cli.send_packets( @dest, :tp_src => 60000 )
      end


      it "should send_packets (tp_dst = 10000)" do
        @cli.should_receive( :sh ).with( /cli -i trema-0 send_packets --ip_src 192.168.0.1 --ip_dst 192.168.0.2 --tp_src 1 --tp_dst 10000 --pps 1 --duration 1 --length 22$/ )
        @cli.send_packets( @dest, :tp_dst => 10000 )
      end


      it "should send packets (inc_tp_src)" do
        @cli.should_receive( :sh ).with( /cli -i trema-0 send_packets --ip_src 192.168.0.1 --ip_dst 192.168.0.2 --tp_src 1 --tp_dst 1 --pps 1 --duration 1 --length 22 --inc_tp_src$/ )
        @cli.send_packets( @dest, :inc_tp_src => true )
      end


      it "should send packets (inc_tp_src = 1)" do
        @cli.should_receive( :sh ).with( /cli -i trema-0 send_packets --ip_src 192.168.0.1 --ip_dst 192.168.0.2 --tp_src 1 --tp_dst 1 --pps 1 --duration 1 --length 22 --inc_tp_src=1$/ )
        @cli.send_packets( @dest, :inc_tp_src => 1 )
      end


      it "should send_packets (pps = 100)" do
        @cli.should_receive( :sh ).with( /cli -i trema-0 send_packets --ip_src 192.168.0.1 --ip_dst 192.168.0.2 --tp_src 1 --tp_dst 1 --pps 100 --duration 1 --length 22$/ )
        @cli.send_packets( @dest, :pps => 100 )
      end


      it "should send packets (duration = 10)" do
        @cli.should_receive( :sh ).with( /cli -i trema-0 send_packets --ip_src 192.168.0.1 --ip_dst 192.168.0.2 --tp_src 1 --tp_dst 1 --pps 1 --duration 10 --length 22$/ )
        @cli.send_packets( @dest, :duration => 10 )
      end


      it "should send packets (length = 1000)" do
        @cli.should_receive( :sh ).with( /cli -i trema-0 send_packets --ip_src 192.168.0.1 --ip_dst 192.168.0.2 --tp_src 1 --tp_dst 1 --pps 1 --duration 1 --length 1000$/ )
        @cli.send_packets( @dest, :length => 1000 )
      end


      it "should send_packets (inc_tp_dst)" do
        @cli.should_receive( :sh ).with( /cli -i trema-0 send_packets --ip_src 192.168.0.1 --ip_dst 192.168.0.2 --tp_src 1 --tp_dst 1 --pps 1 --duration 1 --length 22 --inc_tp_dst$/ )
        @cli.send_packets( @dest, :inc_tp_dst => true )
      end


      it "should send_packets (inc_tp_dst = 65534)" do
        @cli.should_receive( :sh ).with( /cli -i trema-0 send_packets --ip_src 192.168.0.1 --ip_dst 192.168.0.2 --tp_src 1 --tp_dst 1 --pps 1 --duration 1 --length 22 --inc_tp_dst=65534$/ )
        @cli.send_packets( @dest, :inc_tp_dst => 65534 )
      end


      it "should send_packets (inc_payload)" do
        @cli.should_receive( :sh ).with( /cli -i trema-0 send_packets --ip_src 192.168.0.1 --ip_dst 192.168.0.2 --tp_src 1 --tp_dst 1 --pps 1 --duration 1 --length 22 --inc_payload$/ )
        @cli.send_packets( @dest, :inc_payload => true )
      end


      it "should send_packets (inc_payload = 1000)" do
        @cli.should_receive( :sh ).with( /cli -i trema-0 send_packets --ip_src 192.168.0.1 --ip_dst 192.168.0.2 --tp_src 1 --tp_dst 1 --pps 1 --duration 1 --length 22 --inc_payload=1000$/ )
        @cli.send_packets( @dest, :inc_payload => 1000 )
      end


      it "should send_packets (n_pkts = 10)" do
        @cli.should_receive( :sh ).with( /cli -i trema-0 send_packets --ip_src 192.168.0.1 --ip_dst 192.168.0.2 --tp_src 1 --tp_dst 1 --pps 1 --length 22 --n_pkts=10$/ )
        @cli.send_packets( @dest, :n_pkts => 10 )
      end


      it "should raise if both --duration and --n_pkts are specified" do
        expect { @cli.send_packets( @dest, :duration => 10, :n_pkts => 10 ) }.to raise_error( "--duration and --n_pkts are exclusive." )
      end
    end


    context "when resetting stats" do
      it "should reset_stats" do
        @cli.should_receive( :sh ).with( /cli -i trema-0 reset_stats --tx$/ )
        @cli.should_receive( :sh ).with( /cli -i trema-0 reset_stats --rx$/ )
        @cli.reset_stats
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
