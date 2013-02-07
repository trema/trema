#
# Common utility functions.
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


require "trema/dsl"
require "trema/executables"
require "trema/path"
require "trema/process"


module Trema::Util
  def sh cmd
    ENV[ "TREMA_HOME" ] = Trema.home
    puts cmd if $verbose
    unless system( cmd )
      raise "Command '#{ cmd }' failed!"
    end
  end


  def assert_trema_is_built
    unless Trema::Executables.compiled?
      $stderr.puts <<-EOF
ERROR: Trema is not compiled yet!

Please try the following command:
% ./build.rb
EOF
      exit false
    end
  end


  def cleanup session
    # [FIXME] Use session.switch_manager
    sm_pid = File.join( Trema.pid, "switch_manager.pid" )
    if FileTest.exist?( sm_pid )
      Trema::Process.read( sm_pid ).kill!
    end
    session.apps.each do | name, app |
      app.shutdown!
    end
    session.switches.each do | name, switch |
      switch.shutdown!
    end
    session.hosts.each do | name, host |
      host.shutdown!
    end
    session.links.each do | name, link |
      link.delete!
    end
    session.netnss.each do | name, netns |
      netns.shutdown!
    end

    Dir.glob( File.join Trema.pid, "*.pid" ).each do | each |
      Trema::Process.read( each ).kill!
    end
  end


  def running?
    FileTest.exists? Trema::DSL::Context::PATH
  end


  def cleanup_current_session
    begin
      cleanup Trema::DSL::Context.load_current
    ensure
      FileUtils.rm_f Trema::DSL::Context::PATH
    end
  end


  def find_app_by_name name
    # [FIXME] Trema apps does not appear in context.apps. why?
    pid_file = File.join( Trema.pid, "#{ name }.pid" )
    if FileTest.exist?( pid_file )
      Trema::Process.read( pid_file )
    else
      nil
    end
  end


  def find_switch_by_name name
    Trema::DSL::Context.load_current.switches[ name ]
  end


  def find_host_by_name name
    Trema::DSL::Context.load_current.hosts[ name ]
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
