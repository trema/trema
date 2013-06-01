#
# The controller class of tremashark.
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


module Trema
  #
  # Represents an abstract instance to start the tremashark debugging program.
  #
  class Tremashark
    #
    # Runs the tremashark program as a daemon for debugging.
    #
    # @return [Boolean, Nil]
    #   a true or false tremashark exit status code or nil if failed to run.
    #
    def run
      sh "#{ Executables.tremashark } --daemonize"
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
