#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
#
# Copyright (C) 2008-2012 NEC Corporation
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
  class Flow
    attr_reader :dl_type


    def self.parse line
      flow = self.new
      # to simplify parsing
      line.sub!(/actions=.*,.*$/) { | match | match.gsub(/,/,'/') }
      line.strip.split( /[,\s]\s*/ ).each do | each |
        next unless /(.+)=(.+)/=~ each
        name, value = $1, $2
        attr_reader name.to_sym
        if ( /\A\d+\Z/=~ value ) or ( /\A0x.+\Z/=~ value )
          flow.instance_eval "@#{ name }=#{ value }"
        else
          flow.instance_eval "@#{ name }='#{ value }'"
        end
      end
      flow
    end


    def users_flow?
      not ( ( @actions == "drop" and @priority == 0 ) or
            @actions == "CONTROLLER:65535" )
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
