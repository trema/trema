Feature: -P (--pid_dir) option

  -P (--pid_dir) option specifies the location to put pid files

  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "null_controller.rb" with:
      """ruby
      class NullController < Trema::Controller; end
      """

  @sudo
  Scenario: -P option
    Given a directory named "pid"
    When I successfully run `trema run null_controller.rb -P pid -d`
    And sleep 3
    Then a file named "pid/NullController.pid" should exist

  @sudo
  Scenario: --pid_dir option
    Given a directory named "pid"
    When I successfully run `trema run null_controller.rb --pid_dir pid -d`
    And sleep 3
    Then a file named "pid/NullController.pid" should exist

  @sudo
  Scenario: "No such directory" error
    When I run `trema run null_controller.rb -P pid -d`
    Then the exit status should not be 0
    And the stderr should contain:
      """
      No such directory
      """
