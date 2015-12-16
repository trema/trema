Feature: -d (--daemonize) option

  -d (--daemonize) runs a controller as a daemon

  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "null_controller.rb" with:
      """ruby
      class NullController < Trema::Controller; end
      """

  @sudo
  Scenario: -d option
    When I successfully run `trema run null_controller.rb -d`
    Then the command returns immediately

  @sudo
  Scenario: --daemonize option
    When I successfully run `trema run null_controller.rb --daemonize`
    Then the command returns immediately
