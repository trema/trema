Feature: -L (--log_dir) option

  -L (--log_dir) option specifies the location to put log files

  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "null_controller.rb" with:
      """ruby
      class NullController < Trema::Controller; end
      """

  @sudo
  Scenario: -L option
    Given a directory named "log"
    When I successfully run `trema run null_controller.rb -L log -d`
    And sleep 3
    Then a file named "log/NullController.log" should exist

  @sudo
  Scenario: --log_dir option
    Given a directory named "log"
    When I successfully run `trema run null_controller.rb --log_dir log -d`
    And sleep 3
    Then a file named "log/NullController.log" should exist

  @sudo
  Scenario: "No such directory" error
    When I run `trema run null_controller.rb -L log -d`
    Then the exit status should not be 0
    And the stderr should contain:
      """
      No such directory
      """
