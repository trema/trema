Feature: -l (--logging_level) option
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "logger_controller.rb" with:
      """ruby
      class LoggerController < Trema::Controller
        def start(_args)
          logger.debug 'debug message'
          logger.info 'info message'
          logger.warn 'warn message'
          logger.error 'error message'
          logger.fatal 'fatal message'
          logger.unknown 'unknown message'
        end
      end
      """

  @sudo
  Scenario: the default logging level
    When I successfully trema run "logger_controller.rb" with args "-d"
    And the file "LoggerController.log" should not contain "DEBUG -- : debug message"
    And the file "LoggerController.log" should contain "INFO -- : info message"
    And the file "LoggerController.log" should contain "WARN -- : warn message"
    And the file "LoggerController.log" should contain "ERROR -- : error message"
    And the file "LoggerController.log" should contain "FATAL -- : fatal message"
    And the file "LoggerController.log" should contain "ANY -- : unknown message"

  @sudo
  Scenario: --logging_level debug
    When I successfully trema run "logger_controller.rb" with args "--logging_level debug -d"
    And the file "LoggerController.log" should contain "DEBUG -- : debug message"

  @sudo
  Scenario: "Invalid log level" error
    When I trema run "logger_controller.rb" with args "--logging_level hoge"
    Then the exit status should not be 0
    And the stderr should contain:
      """
      Invalid log level: hoge
      """
