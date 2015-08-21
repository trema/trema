Feature: -S (--socket_dir) option

  -S (--socket_dir) option specifies the location to put socket files

  Background:
    Given a file named "null_controller.rb" with:
      """ruby
      class NullController < Trema::Controller; end
      """
  
  @sudo
  Scenario: -S option
    Given a socket directory named "sock"
    When I successfully run `trema run null_controller.rb -S sock -d`
    And I run `sleep 3`
    Then a socket file named "sock/NullController.ctl" should exist
    And a socket file named "sock/trema.NullController.ctl" should exist

  @sudo
  Scenario: --socket_dir option
    Given a socket directory named "sock"
    When I successfully run `trema run null_controller.rb --socket_dir sock -d`
    And I run `sleep 3`
    Then a socket file named "sock/NullController.ctl" should exist
    And a socket file named "sock/trema.NullController.ctl" should exist

  @sudo
  Scenario: "No such directory" error
    When I run `trema run null_controller.rb -S sock -d`
    Then the exit status should not be 0
    And the stderr should contain:
      """
      No such directory
      """
