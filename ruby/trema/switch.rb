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


module Trema
  class Switch
    attr_reader :dpid
    alias :datapath_id :dpid


    def self.inherited subclass
      at_exit { subclass.new( eval ARGV[ 0 ] ).run! }
    end


    def initialize dpid
      @dpid = dpid
    end


    def name
      self.class.to_s.split( "::" ).last
    end


    def controller_connected
      send_message Hello.new
    end


    def features_request xid
      send_message FeaturesReply.new( :datapath_id => @dpid, :transaction_id => xid )
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
