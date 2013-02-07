#
# The controller class of switch daemon.
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
  class SwitchDaemon
    MANDATORY_QUEUES = [ :port_status, :packet_in, :state_notify ]


    #
    # Creates a new abstract representation of SwitchDaemon instance
    # by accepting and validating a hash of queue options.
    #
    # @param [Hash] queues a hash of queue arguments to assign.
    # @option queues [Symbol] :port_status
    #   the controller(s) to receive port status messages.
    # @option queues [Symbol] :packet_in
    #   the controller(s) to receive packet_in messages.
    # @option queues [Symbol] :state_notify
    #   the controller(s) to receive state notification messages.
    #
    def initialize queues
      check_mandatory_options queues
      @queues = queues
    end


    #
    # Constructs switch daemon's options associating one or more controller
    # name to a queue name.
    #
    # @return [Array<String>] the switch daemon's options.
    def options
      all_queues = MANDATORY_QUEUES + [ :vendor ]
      all_queues.collect! do | each |
        queue each
      end.flatten!
    end


    ################################################################################
    private
    ################################################################################


    #
    # @raise [RuntimeError] if a mandatory option is not found.
    #
    def check_mandatory_options queues
      MANDATORY_QUEUES.each do | each |
        raise ":#{ each } is a mandatory option" if queues[ each ].nil?
      end
    end


    #
    # @return [Array<String>] an array of controller name strings.
    #
    def queue queue_type
      return [] unless @queues[ queue_type ]
      controllers = @queues[ queue_type ].split( "," )
      controllers.collect! do | each |
        "#{ queue_type.to_s }::#{ each }"
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
