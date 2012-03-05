#
# trema kill command.
#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
#
# Copyright (C) 2008-2012 NEC Corporation
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


require "optparse"
require "trema/dsl"
require "trema/util"


module Trema
  module Command
    include Trema::Util


    def kill
      options = OptionParser.new
      options.banner = "Usage: trema kill NAME [OPTIONS ...]"

      options.on( "-h", "--help" ) do
        puts options.to_s
        exit 0
      end
      options.on( "-v", "--verbose" ) do
        $verbose = true
      end

      options.parse! ARGV

      context = Trema::DSL::Context.load_current

      # [FIXME] Trema apps does not appear in context.apps. why?
      pid_file = File.join( Trema.pid, "#{ ARGV[ 0 ] }.pid" )
      if FileTest.exist?( pid_file )
        Trema::Process.read( pid_file ).kill!
        return
      end

      host = context.hosts[ ARGV[ 0 ] ]
      if host
        host.shutdown
        return
      end

      switch = context.switches[ ARGV[ 0 ] ]
      if switch
        switch.shutdown
        return
      end

      raise "Unknown name: #{ ARGV[ 0 ] }"

      # [TODO] kill a link by its name. Needs a good naming convension for link.
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
