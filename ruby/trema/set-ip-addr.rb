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


require "trema/action"


module Trema
  #
  # @abstract The base class of {SetIpSrcAddr} and {SetIpDstAddr} action.
  #
  class SetIpAddr < Action
    # @return [IPAddr] the object that holds {#ip_address}'s
    #   internal representation.
    attr_reader :ip_address


    #
    # @private
    #
    def initialize ip_address
      unless ip_address.is_a?( String )
        raise TypeError, "Source IP address must be a String"
      end
      @ip_address = IPAddr.new( ip_address )
    end


    def to_s
      "#{ self.class.to_s }: ip_address=#{ @ip_address }"
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
