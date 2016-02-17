require 'phut'

module Trema
  # The default logger.
  class Logger
    def initialize(name)
      @name = name
      @logger = {}.tap do |list|
        list[:file] = create_file_logger
        list[:stdout] = create_stdout_logger
      end
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
      @logger.values.each { |each| each.__send__ :level=, level }
    end

    def level
      @logger[:file].level
    end

    private

    def output(message_type, message)
      @logger.values.each { |each| each.__send__ message_type, message }
      message
    end

    def create_file_logger
      ::Logger.new("#{Phut.log_dir}/#{@name}.log").tap do |logger|
        logger.level = ::Logger::INFO
      end
    end

    def create_stdout_logger
      $stdout.sync = true
      ::Logger.new($stdout).tap do |logger|
        logger.formatter = proc { |_sev, _dtm, _name, msg| msg + "\n" }
        logger.level = ::Logger::INFO
      end
    end
  end
end
