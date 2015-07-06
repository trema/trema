Feature: OpenFlow1.3
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |

  @sudo
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

  @sudo
  Scenario: send_flow_mod_add
    Given a file named "flow_mod_controller.rb" with:
      """
      class FlowModController < Trema::Controller
        def switch_ready(datapath_id)
          logger.info 'Sending a FlowMod with OpenFlow 1.3 options'
          send_flow_mod_add(
            datapath_id,
            table_id: 0,
            idle_timeout: 180,
            flags: [],
            priority: 1,
            match: Match.new({}),
            instructions: []
          )
          logger.info 'Sent a FlowMod successfully'
        end
      end
      """
    And a file named "trema.conf" with:
      """
      vswitch { datapath_id 0xabc }
      """
    When I successfully run `trema run flow_mod_controller.rb --openflow13 -c trema.conf -d`
    And I run `sleep 5`
    Then the file "FlowModController.log" should contain "Sent a FlowMod successfully"

