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


require "mkmf"


$CFLAGS = "-g -std=gnu99 -D_GNU_SOURCE -fno-strict-aliasing -Wall -Wextra -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wfloat-equal -Wpointer-arith"
$LDFLAGS = "-Wl,-Bsymbolic"


dir_config "trema"
dir_config "openflow"


def error_exit message
  $stderr.puts message
  exit false
end


unless find_library( "rt", "clock_gettime" )
  error_exit <<-EOF
ERROR: librt not found!

Please install libc6-dev with following command:
% sudo apt-get install libc6-dev
EOF
end

unless find_library( "trema", "create_hello" )
  error_exit <<-EOF
ERROR: Trema is not compiled yet!

Please try the following command:
% rake
EOF
end


create_makefile "trema", "trema"

