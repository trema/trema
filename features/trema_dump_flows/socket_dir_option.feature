Feature: -S (--socket_dir) option

  -S (--socket_dir) option specifies the location to find socket files

  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
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
    And I successfully run `trema run flow_mod_controller.rb -c trema.conf -S . -d`
  
  @sudo
  Scenario: -S option
    When I successfully run `trema dump_flows of_switch -S .`
    Then the output should match:
      """
      ^cookie=0x0, duration=\d+\.\d+s, table=0, n_packets=0, n_bytes=0, idle_age=\d+, priority=0 actions=drop
      """

  @sudo
  Scenario: --socket_dir option
    When I successfully run `trema dump_flows of_switch --socket_dir .`
    Then the output should match:
      """
      ^cookie=0x0, duration=\d+\.\d+s, table=0, n_packets=0, n_bytes=0, idle_age=\d+, priority=0 actions=drop
      """

  @sudo
  Scenario: "No such directory" error
    When I run `trema dump_flows of_switch --socket_dir sock`
    Then the exit status should not be 0
    And the stderr should contain:
      """
      No such directory
      """
