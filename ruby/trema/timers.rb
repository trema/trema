#
# Author: Nick Karanatsios <nickkaranatsios@gmail.com>
#
# Copyright (C) 2008-2011 NEC Corporation
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


module Timers
  def self.included base
    base.send :include, TimerMethods
    base.send :extend, TimerMethods
  end


  module TimerMethods
    # needs refactoring to remove module variable
    @@timer_event_handlers = {}
  
  
    def add_timer_event handler, interval, event_type
      @@timer_event_handlers[ handler ] = {
        :interval => interval,
        :rest => interval,
        :event_type => event_type
      }
    end
    alias_method :timer_event, :add_timer_event

  
    def delete_timer_event handler
      @@timer_event_handlers.delete handler 
    end

  
    # shortcut methods
    def add_periodic_timer_event handler, interval
      add_timer_event handler, interval, :periodic
    end
    alias_method :periodic_timer_event, :add_periodic_timer_event

  
    def add_oneshot_timer_event handler, interval
      add_timer_event handler, interval, :oneshot
    end
    alias_method :oneshot_timer_event, :add_oneshot_timer_event
  
  
    def handle_timer_event
      @@timer_event_handlers.each do | handler, data |
        data[ :rest ] -= 1
        if data[ :rest ] <= 0
          __send__ handler
          data[ :rest ] = data[ :interval ] if data[ :event_type ] == :periodic
        end
      end
      @@timer_event_handlers.delete_if do | handler, data |
        data[ :rest ] <= 0 && data[ :event_type ] == :oneshot
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
