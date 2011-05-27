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


require File.join( File.dirname( __FILE__ ), "..", "spec_helper" )
require "trema/dsl/context"
require "trema/dsl/runner"
require "trema/util"


include Trema::Util


describe Trema::Util do
  it "should check the sanity of trema" do
    Trema::Executables.stub!( :compiled? ).and_return( false )
    $stderr.should_receive( :puts ).with( /^ERROR/ )
    lambda do
      sanity_check
    end.should raise_error( SystemExit )
  end


  it "should execute and check the results of a command" do
    lambda do
      sh "NO SUCH COMMAND"
    end.should raise_error( "Command 'NO SUCH COMMAND' failed!" )
  end


  it "should cleanup current session" do
    apps = [ mock( "app 1" ), mock( "app 2" ), mock( "app 1" ) ]
    apps.each do | each |
      each.should_receive( :shutdown! )
    end
    
    switches = [ mock( "switch 1" ), mock( "switch 2" ), mock( "switch 1" ) ]
    switches.each do | each |
      each.should_receive( :shutdown! )
    end

    hosts = [ mock( "switch 1" ), mock( "switch 2" ), mock( "switch 1" ) ]
    hosts.each do | each |
      each.should_receive( :shutdown! )
    end
    
    links = [ mock( "link 1" ), mock( "link 2" ), mock( "link 1" ) ]
    links.each do | each |
      each.should_receive( :down! )
    end

    last_session = mock( "last session" )
    last_session.stub!( :apps ).and_return( apps )
    last_session.stub!( :switches ).and_return( switches )
    last_session.stub!( :hosts ).and_return( hosts )
    last_session.stub!( :links ).and_return( links )
    Trema::DSL::Context.stub!( :load_from ).and_return( last_session )
    
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
