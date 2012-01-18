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


Then /^the total number of tx packets should be:$/ do | table |
  table.hashes[ 0 ].each_pair do | host, n |
    count_packets( `./trema show_stats #{ host } --tx` ).should == n.to_i
  end
end


Then /^the total number of rx packets should be:$/ do | table |
  table.hashes[ 0 ].each_pair do | host, n |
    count_packets( `./trema show_stats #{ host } --rx` ).should == n.to_i
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
