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
require "trema/dsl/configuration"
require "trema/dsl/runner"
require "trema/util"


include Trema::Util


describe Trema::Util do
  it "should assert that trema is built" do
    Trema::Executables.stub!( :compiled? ).and_return( false )
    $stderr.should_receive( :puts ).with( /^ERROR/ )
    expect { assert_trema_is_built }.to raise_error( SystemExit )
  end


  it "should execute and check the results of a command" do
    expect { sh "NO SUCH COMMAND" }.to raise_error( "Command 'NO SUCH COMMAND' failed!" )
  end


  it "should cleanup current session" do
    apps = { "app 1" => mock( "app 1" ), "app 2" => mock( "app 2" ), "app 3" => mock( "app 3" ) }
    apps.each do | name, app |
      app.should_receive( :shutdown! )
    end

    switches = { "switch 1" => mock( "switch 1" ), "switch 2" => mock( "switch 2" ), "switch 3" => mock( "switch 3" ) }
    switches.each do | name, switch |
      switch.should_receive( :shutdown! )
    end

    hosts = { "host 1" => mock( "host 1" ), "host 2" => mock( "host 2" ), "host 3" => mock( "host 3" ) }
    hosts.each do | name, host |
      host.should_receive( :shutdown! )
    end

    links = { "link 1" => mock( "link 1" ), "link 2" => mock( "link 2" ), "link 3" => mock( "link 3" ) }
    links.each do | name, link |
      link.should_receive( :delete! )
    end

    last_session = mock( "last session" )
    last_session.stub!( :apps ).and_return( apps )
    last_session.stub!( :switches ).and_return( switches )
    last_session.stub!( :hosts ).and_return( hosts )
    last_session.stub!( :links ).and_return( links )
    last_session.stub!( :netnss ).and_return( {} )
    Trema::DSL::Context.stub!( :load_current ).and_return( last_session )

    pid_files = [ mock( "PID file #0" ), mock( "PID file #1" ), mock( "PID file #2" ) ]
    Dir.stub!( :glob ).and_return( pid_files )

    process = mock( "process" )
    process.should_receive( :kill! ).exactly( 3 ).times
    Trema::Process.should_receive( :read ).with( pid_files[ 0 ] ).once.ordered.and_return( process )
    Trema::Process.should_receive( :read ).with( pid_files[ 1 ] ).once.ordered.and_return( process )
    Trema::Process.should_receive( :read ).with( pid_files[ 2 ] ).once.ordered.and_return( process )

    cleanup_current_session
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
