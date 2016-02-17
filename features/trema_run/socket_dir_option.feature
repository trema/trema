Feature: -S (--socket_dir) option

  -S (--socket_dir) option specifies the location to put socket files

  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
    And a file named "null_controller.rb" with:
      """ruby
      class NullController < Trema::Controller; end
      """
  
  @sudo
  Scenario: -S option
    When I successfully run `trema run null_controller.rb -S . -d`
    And sleep 3
    Then a socket file named "NullController.ctl" should exist

  @sudo
  Scenario: --socket_dir option
    When I successfully run `trema run null_controller.rb --socket_dir . -d`
    And sleep 3
    Then a socket file named "NullController.ctl" should exist

  @sudo
  Scenario: "No such directory" error
    When I run `trema run null_controller.rb -S sock -d`
    Then the exit status should not be 0
    And the stderr should contain:
      """
      No such directory
      """
