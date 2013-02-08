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


require "mkmf"


$CFLAGS = "-g -std=gnu99 -D_GNU_SOURCE -fno-strict-aliasing -Wall -Wextra -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wfloat-equal -Wpointer-arith"
$LDFLAGS = "-Wl,-Bsymbolic"


dir_config "trema"
dir_config "openflow"


def error_exit message
  $stderr.puts message
  exit false
end


def error_lib_missing lib, package
  return <<-EOF
ERROR: #{ lib } not found!

Please install #{ package } with following command:
% sudo apt-get install #{ package }
EOF
end


unless find_library( "pthread", "pthread_create" )
  error_exit error_lib_missing( "libpthread", "libc6-dev" )
end

unless find_library( "rt", "clock_gettime" )
  error_exit error_lib_missing( "librt", "libc6-dev" )
end

unless find_library( "dl", "dlopen" )
  error_exit error_lib_missing( "libdl", "libc6-dev" )
end

unless find_library( "sqlite3", "sqlite3_open" )
  error_exit error_lib_missing( "libsqlite3", "libsqlite3-dev" )
end

unless find_library( "trema", "create_hello" )
  error_exit <<-EOF
ERROR: Trema is not compiled yet!

Please try the following command:
% ./build.rb
EOF
end


$CFLAGS << " -Werror" # must be added after find_library
create_makefile "trema", "trema"


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
