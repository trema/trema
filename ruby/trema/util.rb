#
# Common utility functions.
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


require "trema/dsl"
require "trema/executables"
require "trema/path"
require "trema/process"


module Trema::Util
  def sanity_check
    unless Trema::Executables.compiled?
      $stderr.puts <<-EOF
ERROR: Trema is not compiled yet!

Please try the following command:
% ./build.rb
EOF
      exit false
    end
  end


  def sh cmd
    ENV[ "TREMA_HOME" ] = Trema.home
    puts cmd if $verbose
    unless system( cmd )
      raise "Command '#{ cmd }' failed!"
    end
  end


  def cleanup session
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

    Dir.glob( File.join Trema.pid_directory, "*.pid" ).each do | each |
      Trema::Process.read( each ).kill!
    end

    FileUtils.rm_f Trema::DSL::Parser::CURRENT_CONTEXT
  end


  def cleanup_current_session
    cleanup Trema::DSL::Parser.new.load_current
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
