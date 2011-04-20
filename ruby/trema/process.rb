#
# Trema process.
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


module Trema
  class Process
    def self.read pid_file
      return new( pid_file )
    end


    def initialize pid_file
      @pid_file = pid_file
      begin
        @pid = IO.read( @pid_file ).chomp.to_i
        @uid = File.stat( @pid_file ).uid
      rescue
        @pid_file = nil
      end
    end


    def kill!
      return if @pid_file.nil?
      return if not alive?
      if @uid == 0
        sh "sudo kill #{ @pid } 2>/dev/null" rescue nil
      else
        sh "kill #{ @pid } 2>/dev/null" rescue nil
      end
      loop do
        return unless FileTest.exists?( @pid_file )
      end
    end


    ################################################################################
    private
    ################################################################################


    def alive?
      not `ps ax | grep -E "^[[:blank:]]*#{ @pid }"`.empty?
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
