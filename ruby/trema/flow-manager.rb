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

require "trema/controller"
require "trema/executables"

module Trema
  module FlowManager

    def self.handler name
    	#Do nothing.
    end

    #
    # @!method flow_manager_setup_reply( status, path )
    #
    # @abstract Override this to implement a custom handler.
    #
    # @param [Integer] status
    #   the datapath ID of disconnected OpenFlow switch.
    #
	  handler :flow_manager_setup_reply
	
	  #
    # @!method flow_manager_teardown_reply( reason, path )
    #
    # @abstract Override this to implement a custom handler.
    #
    # @param [Integer] reason
    #   the datapath ID of disconnected OpenFlow switch.
    #
	  handler :flow_manager_teardown_reply

    #
    # @!method start
    # Initialization before start_trema() call.
    # This method will be implicitly called inside Controller#run! between init_trema() and start_trema() calls.
    #
    def start
      Flow_manager.initialize() 
    end
    
    #
    # @overload shutdown!
    #  Shutdown controller.
    #
    def shutdown!     
      super
      sleep 1
      Flow_manager.finalize()
    end
  end
end
