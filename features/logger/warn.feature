Feature: logger#warn
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "hello.rb" with:
      """ruby
      class Hello < Trema::Controller
        def start(_args)
          logger.warn 'Konnichi Wa'
        end
      end
      """

  @sudo
  Scenario: the default logging level
    When I trema run "hello.rb" interactively
    And I trema killall "Hello"
    Then the output should contain "Konnichi Wa"
    And the file "Hello.log" should contain "WARN -- : Konnichi Wa"

  @sudo
  Scenario: --logging_level warn
    When I trema run "hello.rb" with args "--logging_level warn" interactively
    And I trema killall "Hello"
    Then the output should contain "Konnichi Wa"
    And the file "Hello.log" should contain "WARN -- : Konnichi Wa"

  @sudo
  Scenario: --logging_level error
    When I trema run "hello.rb" with args "--logging_level error" interactively
    And I trema killall "Hello"
    Then the output should not contain "Konnichi Wa"
    And the file "Hello.log" should not contain "WARN -- : Konnichi Wa"

  @sudo
  Scenario: -v
    When I trema "-v" run "hello.rb" interactively
    And I trema killall "Hello"
    Then the output should contain "Konnichi Wa"
    And the file "Hello.log" should contain "WARN -- : Konnichi Wa"

  @sudo
  Scenario: --verbose
    When I trema "--verbose" run "hello.rb" interactively
    And I trema killall "Hello"
    Then the output should contain "Konnichi Wa"
    And the file "Hello.log" should contain "WARN -- : Konnichi Wa"
