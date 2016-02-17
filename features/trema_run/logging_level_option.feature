Feature: -l (--logging_level) option
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
          logger.debug 'Konnichi Wa'
        end
      end
      """

  @sudo
  Scenario: the default logging level
    When I trema run "hello.rb"
    And the file "Hello.log" should not contain "DEBUG -- : Konnichi Wa"

  @sudo
  Scenario: --logging_level debug
    When I run `trema run hello.rb --logging_level debug -d`
    And sleep 3
    And the file "Hello.log" should contain "DEBUG -- : Konnichi Wa"

  @sudo
  Scenario: "Invalid log level" error
    When I run `trema run hello.rb --logging_level hoge -d`
    Then the exit status should not be 0
    And the stderr should contain:
      """
      Invalid log level: hoge
      """
