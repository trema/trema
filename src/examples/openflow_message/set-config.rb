#
# A test example program to send a configurable number of OFPT_SET_CONFIG
# messages.
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


require "example"


class SetConfigController < Controller
  include Example


  class << self
    def run args
      usage unless Example.options_parse args
    end


    def usage
      puts Example.cmd_usage
      puts "Send count number of set-config messages to datapath_id."
      exit false
    end
  end


  def switch_ready msg_datapath_id
    may_raise_error msg_datapath_id
    send_nr_msgs SetConfig
  end
end


SetConfigController.run ARGV


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
