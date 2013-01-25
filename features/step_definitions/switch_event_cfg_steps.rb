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


Given /^I trema run "(.+)" with (\d+) switches$/ do | controller, dpid_max |
  steps %Q{
    Given a file named "nw_dsl.conf" with:
      """
      1.upto( #{dpid_max} ).each do |sw|
        vswitch { datapath_id "%#x" % sw }
      end
      """
    Given I run `trema run #{controller} -c nw_dsl.conf -d`
    Given wait until "#{controller.split('/').last}" is up
  }
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
