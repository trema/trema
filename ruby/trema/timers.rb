module Timers


  def self.included base
    base.send :include, TimerMethods
    base.send :extend, TimerMethods
  end


  module TimerMethods

    # needs refactoring to remove module variable
    @@timer_event_handlers = {}
  
  
    def add_timer_event handler, interval, event_type
      @@timer_event_handlers[ handler ] = { :interval => interval, 
        :rest => interval, :event_type => event_type  }
    end

  
    def delete_timer_event handler
      @@timer_event_handlers.delete handler 
    end

  
    # shortcut methods
    def add_periodic_timer_event handler, interval
      add_timer_event handler, interval, :periodic
    end

  
    def add_oneshot_timer_event handler, interval
      add_timer_event handler, interval, :oneshot
    end
  
  
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
