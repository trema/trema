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


require "trema/monkey-patch/module"
require "trema/monkey-patch/string"
require "trema/path"


#
# Holds a list of executables found in Trema.objects directory.
#
class Trema::Executables
  class << self
    #
    # Cycles through a list of file names testing if there are executable or
    # not.
    #
    # @return [FalseClass, Array]
    #   false if a file name is not an executable program or a list of all
    #   file names that are.
    #
    def compiled?
      @list.each do | each |
        return false unless FileTest.executable?( __send__ each )
      end
    end


    ############################################################################
    private
    ############################################################################


    #
    # Adds the name to a list.
    #
    def add name
      @list ||= []
      @list << name
    end


    #
    # Defines a class method that returns the full path name of an executable
    # program constructed from its relative path. It also adds the
    # class method name to an array.
    #
    # @example
    #   path "openvswitch/bin/ovs-ofctl"
    #
    # @param [String] path
    #   the relative path to an executable program.
    #
    # @return [Array] a list of a class method to access each executable program.
    #
    def path path
      name = File.basename( path ).underscore
      define_class_method( name ) do
        File.join Trema.objects, path
      end
      add name
    end
  end


  path "openvswitch/bin/ovs-ofctl"
  path "openvswitch/bin/ovs-openflowd"
  path "packetin_filter/packetin_filter"
  path "phost/cli"
  path "phost/phost"
  path "switch_manager/switch"
  path "switch_manager/switch_manager"
  path "topology/topology"
  path "tremashark/packet_capture"
  path "tremashark/stdin_relay"
  path "tremashark/syslog_relay"
  path "tremashark/tremashark"
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
