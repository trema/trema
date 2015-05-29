module Trema
  module MonkeyPatch
    module Integer
      # to_hex etc.
      module BaseConversions
        def to_hex
          format '%#x', self
        end
      end
    end
  end
end
