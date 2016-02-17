Feature: dump_flows
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "noop_controller.rb" with:
      """ruby
      class NoopController < Trema::Controller; end
      """
    And a file named "flow_mod_controller.rb" with:
      """ruby
      class FlowModController < Trema::Controller
        def switch_ready(datapath_id)
          send_flow_mod_add(datapath_id, match: Match.new)
        end
      end
      """
    And a file named "trema.conf" with:
      """ruby
      vswitch('of_switch') { datapath_id 0xabc }
      """

  @sudo
  Scenario: dump_flows (no flow entries)
    Given I trema run "noop_controller.rb" with the configuration "trema.conf"
    When I successfully run `trema dump_flows of_switch`
    Then the output from "trema dump_flows of_switch" should contain exactly ""

  @sudo
  Scenario: dump_flows (one flow entry)
    Given I trema run "flow_mod_controller.rb" with the configuration "trema.conf"
    When I successfully run `trema dump_flows of_switch`
    Then the output should match:
      """
      ^cookie=0x0, duration=\d+\.\d+s, table=0, n_packets=0, n_bytes=0, idle_age=\d+, priority=0 actions=drop
      """

  @sudo
  Scenario: dump_flows OpenFlow 1.3 (no flow entries)
    Given I use OpenFlow 1.3
    And I trema run "noop_controller.rb" with the configuration "trema.conf"
    When I successfully run `trema dump_flows of_switch`
    Then the output from "trema dump_flows of_switch" should contain exactly ""

  @sudo
  Scenario: dump_flows OpenFlow 1.3 (one flow entry)
    Given I use OpenFlow 1.3
    And I trema run "flow_mod_controller.rb" with the configuration "trema.conf"
    When I successfully run `trema dump_flows of_switch`
    Then the output should match:
      """
      ^cookie=0x0, duration=\d+\.\d+s, table=0, n_packets=0, n_bytes=0, priority=0 actions=drop
      """
