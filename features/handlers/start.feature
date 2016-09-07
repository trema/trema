Feature: start handler
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And I use a fixture named "event_logger"

  @sudo
  Scenario: invoke start handler
    When I successfully trema run "event_logger.rb" with args "-d"
    Then the file "EventLogger.log" should contain "args = []"

  @sudo
  Scenario: invoke start handler with args
    When I successfully trema run "event_logger.rb" with args "-d -- arg0 arg1 arg2"
    Then the file "EventLogger.log" should contain "args = [arg0, arg1, arg2]"

  @sudo
  Scenario: invoke start handler (OpenFlow 1.3)
    When I successfully trema run "event_logger.rb" with args "--openflow13 -d"
    Then the file "EventLogger.log" should contain "args = []"

  @sudo
  Scenario: invoke start handler with args (OpenFlow 1.3)
    When I successfully trema run "event_logger.rb" with args "--openflow13 -d -- arg0 arg1 arg2"
    Then the file "EventLogger.log" should contain "args = [arg0, arg1, arg2]"
