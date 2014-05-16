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

require 'fileutils'
require 'trema/dsl/configuration'
require 'trema/dsl/syntax'
require 'trema/path'

module Trema
  module DSL
    class Context
      PATH = File.join(Trema.tmp, '.context')

      def self.load_current
        if FileTest.exists?(PATH)
          Marshal.load(IO.read PATH)
        else
          Configuration.new
        end
      end

      def initialize(config)
        @config = config
      end

      #
      # Dumps a {Configuration} object to <code>PATH</code>
      #
      # @example
      #   context.dump
      #
      # @return [Configuration]
      #
      def dump
        File.open(PATH, 'w') do | f |
          f.print Marshal.dump(@config)
        end
        @config
      end
    end
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
