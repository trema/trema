module Trema
  module MonkeyPatch
    module Integer
      # sec, hour, etc.
      module Durations
        def sec
          self
        end
      end
    end
  end
end
