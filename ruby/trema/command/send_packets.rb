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

require 'trema/cli'
require 'trema/dsl'
require 'trema/util'

module Trema
  module Command
    include Trema::Util

    def trema_send_packets(source_name, dest_name, options)
      source = find_host_by_name(source_name)
      fail "unknown host: #{ source_name }" if source.nil?

      dest = find_host_by_name(dest_name)
      fail "unknown host: #{ dest_name }" if dest.nil?

      Trema::Cli.new(source).send_packets(dest, options)
    end
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
