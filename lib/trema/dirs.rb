require 'tmpdir'

# OpenFlow controller framework.
module Trema
  # Default values.
  module Defaults
    DEFAULT_LOG_DIR = ENV['TREMA_LOG_DIR'] || Dir.tmpdir
    DEFAULT_PID_DIR = ENV['TREMA_PID_DIR'] || Dir.tmpdir
    DEFAULT_SOCKET_DIR = ENV['TREMA_SOCKET_DIR'] || Dir.tmpdir
  end
  Defaults.freeze

  include Defaults
end
