require 'trema/monkey_patch/integer/base_conversions'
require 'trema/monkey_patch/integer/durations'

# Monkey patch for Integer class.
class Integer
  include Trema::MonkeyPatch::Integer::BaseConversions
  include Trema::MonkeyPatch::Integer::Durations
end
