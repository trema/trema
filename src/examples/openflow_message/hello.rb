#
# A test example program to send a OFPT_HELLO message.
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


class HelloController < Controller
  def start
    if ARGV.size != 1
      STDERR.puts "Usage: #{ File.basename __FILE__ } COUNT"
      shutdown!
    end
    @count = ARGV[ 0 ].to_i
  end


  def switch_ready datapath_id
    @count.times do
      send_message datapath_id, Hello.new
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
