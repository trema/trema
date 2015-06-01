require 'trema/controller'

describe Trema::Controller do
  describe '::DEFAULT_TCP_PORT' do
    Then { Trema::Controller::DEFAULT_TCP_PORT == 6653 }
  end
end
