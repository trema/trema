Feature: OpenFlow1.3
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |

  @sudo @announce
  Scenario: trema run with --openflow13 option
    Given a file named "null_controller.rb" with:
      """
      class NullController < Trema::Controller; end
      """
    And a file named "trema.conf" with:
      """
      vswitch { datapath_id 0xabc }
      """
    When I successfully run `trema -v run null_controller.rb --openflow13 -c trema.conf -d`
    And I run `sleep 5`
    Then the output should contain "protocols=OpenFlow13"
