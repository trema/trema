#
# Executes a block exclusively.
#
#
# == Example:
#
#  Blocker.start do
#    # do smething here.
#  end
#
#
# == Credit:
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


class Blocker
  class BlockerError < RuntimeError; end


  PATH = "/tmp/cruise.lock"


  def self.start &code_block
    begin
      block
      code_block.call
    rescue BlockerError, Errno::EACCES
      $stderr.puts "Another process is already running. Please wait for a while."
      sleep 10
      retry
    ensure
      release
    end
  end


  def self.block
    lock = File.open( PATH, "a+" )
    locked = lock.flock( File::LOCK_EX | File::LOCK_NB )
    unless locked
      lock.close
      raise BlockerError
    end
  end


  def self.release
    File.open( PATH, "w" ) do | lock |
      lock.flock( File::LOCK_UN | File::LOCK_NB )
      lock.close
      File.delete lock.path
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
