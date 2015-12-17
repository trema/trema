Feature: --all option

  --all option kills all known trema processes

  @sudo
  Scenario: killall --all
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "null_controller.rb" with:
      """ruby
      class NullController < Trema::Controller; end
      """
    And a file named "void_controller.rb" with:
      """ruby
      class VoidController < Trema::Controller; end
      """
    And I successfully run `trema run null_controller.rb -d`
    And I successfully run `trema run void_controller.rb -p 6654 -d`
    When I successfully run `trema killall --all`
    Then the following files should not exist:
      | NullController.pid |
      | VoidController.pid |
