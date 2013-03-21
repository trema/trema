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


require "trema/path"


def wait_until_all_pid_files_are_deleted timeout = 10
  elapsed = 0
  loop do
    raise "Failed to clean up remaining processes." if elapsed > timeout
    break if Dir.glob( File.join( Trema.pid, "*.pid" ) ).empty?
    sleep 1
    elapsed += 1
  end
  sleep 1
end


Before do
  @aruba_timeout_seconds = 10
  run "trema killall"
  wait_until_all_pid_files_are_deleted
end


Before( "@slow_process" ) do
  @aruba_io_wait_seconds = 1
end


After do
  run "trema killall"
  wait_until_all_pid_files_are_deleted
  processes.clear
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
