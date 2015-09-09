Feature: -S (--socket_dir) option

  -S (--socket_dir) option specifies the location to find socket files

  @sudo
  Scenario: -S option
    Given a file named "null_controller.rb" with:
      """ruby
      class NullController < Trema::Controller; end
      """
    And I successfully run `trema run null_controller.rb -d`
    When I successfully run `trema killall NullController -S .`
    And the file "NullController.pid" should not exist

  @sudo
  Scenario: --socket_dir option
    Given a file named "null_controller.rb" with:
      """ruby
      class NullController < Trema::Controller; end
      """
    And I successfully run `trema run null_controller.rb -d`
    When I successfully run `trema killall NullController --socket_dir .`
    And the file "NullController.pid" should not exist

  @sudo
  Scenario: "Controller process does not exist" error
    Given a file named "null_controller.rb" with:
      """ruby
      class NullController < Trema::Controller; end
      """
    And I successfully run `trema run null_controller.rb -d`
    When I run `trema killall NullController -S /tmp`
    Then the exit status should not be 0
    And the output should contain:
    """
    Controller process "NullController" does not exist.
    """
    And the file "NullController.pid" should exist
