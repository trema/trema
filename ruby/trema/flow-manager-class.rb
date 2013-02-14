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
require "trema/executables"
require "singleton"

module Trema
  class FlowManagerClass    
    include Singleton

    def initialize
      pid_file = Trema.pid+"/flow_manager.pid"
      unless File.exist?(pid_file)
        sh "#{ Trema::Executables.flow_manager } --daemonize"
      else
        puts "flow_manager is already runnning"
      end
    end
  end
end
