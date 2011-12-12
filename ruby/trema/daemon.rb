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


require "trema/monkey-patch/string"
require "trema/process"


module Trema
  module Daemon
    #
    # Kills running daemon process
    #
    # @example
    #   daemon.shutdown!
    #
    # @return [undefined]
    #
    def shutdown!
      Trema::Process.read( pid_file, name ).kill!
    end


    def pid_file
      prefix = self.class.name.demodulize.underscore
      infix = if self.respond_to? :daemon_id
                self.__send__ :daemon_id
              else
                name
              end
      File.join Trema.tmp, "#{ prefix }.#{ infix }.pid"
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
