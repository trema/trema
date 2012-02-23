#
# The controller class of switch daemon.
#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
#
# Copyright (C) 2008-2012 NEC Corporation
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
    def initialize queues
      check_mandatory_options queues
      @queues = queues
    end


    def options
      [ portstatus_queue, packetin_queue, statenotify_queue, vendor_queue ]
    end


    ################################################################################
    private
    ################################################################################


    def check_mandatory_options queues
      [ :port_status, :packet_in, :state_notify ].each do | each |
        raise ":#{ each } is a mandatory option" if queues[ each ].nil?
      end
    end


    def portstatus_queue
      "port_status::#{ @queues[ :port_status ] }"
    end


    def packetin_queue
      "packet_in::#{ @queues[ :packet_in ] }"
    end


    def statenotify_queue
      "state_notify::#{ @queues[ :state_notify ] }"
    end


    def vendor_queue
      "vendor::#{ @queues[ :vendor ] }"
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
