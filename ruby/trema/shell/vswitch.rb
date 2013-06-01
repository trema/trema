#
# vswitch command of Trema shell.
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


require "trema/dsl"


module Trema
  module Shell
    def vswitch name = nil, &block
      raise "Not in Trema shell" if $config.nil?
      raise "No dpid given" if name.nil? and block.nil?

      stanza = DSL::Vswitch.new( name )
      stanza.instance_eval &block if block
      OpenVswitch.new( stanza, $config.port ).restart!

      $context.dump

      true
    end
    module_function :vswitch
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
