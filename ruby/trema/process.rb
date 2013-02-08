#
# Trema process.
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


module Trema
  #
  # A class responsible for terminating processes.
  #
  class Process
    #
    # @overload read(pid_file)
    #   Reads a process identification file and saves the process name
    #   process id and effective user id. The process name is inferred from the
    #   +:pid_file+.
    #   @param [String] pid_file the full path name of the pid file.
    #
    # @overload read(pid_file, name)
    #   Reads a process identification file and saves the process name
    #   process id and effective user id.
    #   @param [String] pid_file the full path name of the pid file.
    #   @param [String] name the process name.
    #
    # @return [Process] the object that encapsulates the process details.
    #
    def self.read pid_file, name = nil
      name = File.basename( pid_file, ".pid" ) if name.nil?
      return new( pid_file, name )
    end


    def initialize pid_file, name
      @name = name
      @pid_file = pid_file
      begin
        @pid = IO.read( @pid_file ).chomp.to_i
        @uid = File.stat( @pid_file ).uid
      rescue
        @pid_file = nil
      end
    end


    #
    # kills an active process.
    #
    # @raise [RuntimeError] if failed to kill the process after a
    #   maximum number of attempts.
    #
    # @return [void]
    #
    def kill!
      return if @pid_file.nil?
      return if dead?
      puts "Shutting down #{ @name }..." if $verbose
      10.times do
        if @uid == 0
          sh "sudo kill #{ @pid } 2>/dev/null" rescue nil
        else
          sh "kill #{ @pid } 2>/dev/null" rescue nil
        end
        return
        # return if dead?
      end
      raise "Failed to shut down #{ @name }"
    end


    ################################################################################
    private
    ################################################################################


    #
    # @return [Boolean] whether a process is not alive or not.
    #
    def dead?
      `ps ax | grep -E "^[[:blank:]]*#{ @pid }"`.empty?
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
