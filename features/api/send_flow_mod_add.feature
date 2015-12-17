Feature: send_flow_mod_add
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "trema.conf" with:
      """
      vswitch { datapath_id 0xabc }
      """

  @sudo
  Scenario: Controller#send_flow_mod_add (OpenFlow 1.0)
    Given a file named "flow_mod_controller10.rb" with:
      """
      class FlowModController10 < Trema::Controller
        def switch_ready(datapath_id)
          logger.info 'Sending a FlowMod with OpenFlow 1.0 options'
          send_flow_mod_add(
            datapath_id,
            actions: [],
            buffer_id: 0,
            command: :add,
            flags: [],
            hard_timeout: 0,
            idle_timeout: 0,
            match: Match.new(),
            out_port: 0,
            priority: 0
          )
          logger.info 'Sent a FlowMod successfully'
        end
      end
      """
    When I successfully run `trema run flow_mod_controller10.rb -c trema.conf -d`
    And sleep 5
    Then the file "FlowModController10.log" should contain "Sent a FlowMod successfully"

  @sudo
  Scenario: Controller#send_flow_mod_add (OpenFlow 1.3)
    Given a file named "flow_mod_controller13.rb" with:
      """
      class FlowModController13 < Trema::Controller
        def switch_ready(datapath_id)
          logger.info 'Sending a FlowMod with OpenFlow 1.3 options'
          send_flow_mod_add(
            datapath_id,
            buffer_id: 0,
            command: :add,
            flags: [],
            hard_timeout: 0,
            idle_timeout: 0,
            instructions: [],
            match: Match.new(),
            out_port: 0,
            priority: 0,
            table_id: 0
          )
          logger.info 'Sent a FlowMod successfully'
        end
      end
      """
    When I successfully run `trema run flow_mod_controller13.rb --openflow13 -c trema.conf -d`
    And sleep 5
    Then the file "FlowModController13.log" should contain "Sent a FlowMod successfully"

