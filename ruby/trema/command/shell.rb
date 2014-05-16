#
# trema shell command.
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

require 'irb'
require 'trema/util'

include Trema::Util

module Trema
  module Command
    def shell
      undef :kill

      require 'tempfile'
      require 'trema'
      require 'trema/shell'
      f = Tempfile.open('irbrc')
      f.print <<EOF
include Trema::Shell
ENV[ "TREMA_HOME" ] = Trema.home
$config = Trema::DSL::Configuration.new
$context = Trema::DSL::Context.new( $config )
EOF
      f.close
      load f.path
      IRB.start
    ensure
      cleanup $config
    end
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
