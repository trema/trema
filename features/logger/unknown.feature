Feature: Trema::Controller#logger.unknown
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
          logger.unknown 'Konnichi Wa'
        end
      end
      """

  @sudo
  Scenario: the default logging level
    When I trema run "hello.rb" interactively
    And sleep 2
    And I trema killall "Hello"
    Then the output should contain "Konnichi Wa"
    And the file "Hello.log" should contain "ANY -- : Konnichi Wa"

  @sudo
  Scenario: --logging_level unknown
    When I run `trema run hello.rb --logging_level unknown` interactively
    And sleep 2
    And I trema killall "Hello"
    Then the output should contain "Konnichi Wa"
    And the file "Hello.log" should contain "ANY -- : Konnichi Wa"

  @sudo
  Scenario: -v
    When I run `trema -v run hello.rb` interactively
    And sleep 2
    And I trema killall "Hello"
    Then the output should contain "Konnichi Wa"
    And the file "Hello.log" should contain "ANY -- : Konnichi Wa"

  @sudo
  Scenario: --verbose
    When I run `trema --verbose run hello.rb` interactively
    And sleep 2
    And I trema killall "Hello"
    Then the output should contain "Konnichi Wa"
    And the file "Hello.log" should contain "ANY -- : Konnichi Wa"
