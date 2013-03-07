#
# Common extendable functionality for all openflow message examples.
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


module Example
  class << self
    attr_accessor :count, :datapath_id


    def options_parse args
      case args.length
      when 2
        @datapath_id = args[ 0 ] =~ /^0x/ ? args[ 0 ].hex : args[ 0 ].to_i
        @count = args[ 1 ].to_i
      else
        return false
      end
    end


    def cmd_usage
      "Usage: #{ File.basename __FILE__ } datapath_id, count"
    end
  end


  def may_raise_error msg_datapath_id
    raise ArgumentError, "Given datapath_id does not match configured datapath_id" if msg_datapath_id != Example::datapath_id
  end


  def send_nr_msgs kclass
    Example::count.times do
      self.send_message Example::datapath_id, kclass.new
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
