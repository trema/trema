Feature: dump_flows
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "trema.conf" with:
      """ruby
      vswitch { datapath_id 0xabc }
      """

  @sudo
  Scenario: dump_flows (no flow entries)
    Given a file named "noop_controller.rb" with:
      """ruby
      class NoopController < Trema::Controller; end
      """
    And I trema run "noop_controller.rb" with the configuration "trema.conf"
    When I successfully run `trema dump_flows 0xabc`
    Then the output from "trema dump_flows 0xabc" should not contain "actions=drop"

  @sudo
  Scenario: dump_flows (one flow entry)
    Given a file named "flow_mod_controller.rb" with:
      """ruby
      class FlowModController10 < Trema::Controller
        def switch_ready(datapath_id)
          send_flow_mod_add(datapath_id, match: Match.new)
        end
      end
      """
    And I trema run "flow_mod_controller.rb" with the configuration "trema.conf"
    When I successfully run `trema dump_flows 0xabc`
    Then the output from "trema dump_flows 0xabc" should contain "actions=drop"
