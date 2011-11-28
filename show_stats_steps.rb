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


Then /^the stats of "([^"]*)" should be:$/ do | host, string |
  step %{I try to run "./trema show_stats #{ host }" (log = "show_stats.log")}
  step %{the content of "show_stats.log" should be:}, string
end


Then /^the tx stats of "([^"]*)" should be:$/ do | host, string |
  step %{I try to run "./trema show_stats #{ host } --tx" (log = "show_stats.log")}
  step %{the content of "show_stats.log" should be:}, string
end


Then /^the rx stats of "([^"]*)" should be:$/ do | host, string |
  step %{I try to run "./trema show_stats #{ host } --rx" (log = "show_stats.log")}
  step %{the content of "show_stats.log" should be:}, string
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
