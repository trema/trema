require 'active_support/core_ext/module/attribute_accessors'
require 'logger'
require 'phut'

# Base module
module Trema
  mattr_accessor :logger

  # The default logger.
  class Logger
    LOGGING_LEVELS = { debug: ::Logger::DEBUG,
                       info: ::Logger::INFO,
                       warn: ::Logger::WARN,
                       error: ::Logger::ERROR,
                       fatal: ::Logger::FATAL,
                       unknown: ::Logger::UNKNOWN }.freeze

    def initialize(name)
      @name = name
    end

    def unknown(message)
      output :unknown, message
    end

    def fatal(message)
      output :fatal, message
    end

    def error(message)
      output :error, message
    end

    def warn(message)
      output :warn, message
    end

    def info(message)
      output :info, message
    end

    def debug(message)
      output :debug, message
    end

    def level=(level)
      @logger ||= create_logger
      @logger.values.each do |each|
        each.__send__ :level=, LOGGING_LEVELS.fetch(level.to_sym)
      end
    rescue KeyError
      raise(ArgumentError, "Invalid log level: #{level}")
    end

    def level
      @logger ||= create_logger
      @logger[:file].level
    end

    private

    def output(message_type, message)
      @logger ||= create_logger
      @logger.values.each { |each| each.__send__ message_type, message }
      message
    end

    def create_logger
      {}.tap do |list|
        list[:file] = create_file_logger
        list[:stderr] = create_stderr_logger
      end
    end

    def create_file_logger
      ::Logger.new("#{Phut.log_dir}/#{@name}.log").tap do |logger|
        logger.level = ::Logger::INFO
      end
    end

    def create_stderr_logger
      ::Logger.new($stderr).tap do |logger|
        logger.formatter = proc { |_sev, _dtm, _name, msg| msg + "\n" }
        logger.level = ::Logger::INFO
      end
    end
  end
end
