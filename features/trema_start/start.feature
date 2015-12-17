Feature: start
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "switch_ready_controller.rb" with:
      """ruby
      class SwitchReadyController < Trema::Controller
        def start(_args)
          @switch_ready_invoked = 0
        end

        def switch_ready(dpid)
          if @switch_ready_invoked > 0
            logger.info "Switch #{dpid.to_hex} connected again."
          else
            logger.info "Switch #{dpid.to_hex} connected."
          end
          @switch_ready_invoked += 1
        end
      end
      """
    And a file named "trema.conf" with:
      """ruby
      vswitch { datapath_id 0xabc }
      vhost('host1') { ip '192.168.0.1' }
      link '0xabc', 'host1'
      """
    And I successfully run `trema run switch_ready_controller.rb -c trema.conf -d`

  @sudo
  Scenario: stop and start a switch
    Given I successfully run `trema stop 0xabc`
    And sleep 3
    When I successfully run `trema start 0xabc`
    And sleep 10
    Then the file "SwitchReadyController.log" should contain:
      """
      Switch 0xabc connected again.
      """

  @sudo
  Scenario: stop and start host_name
    Given I successfully run `trema stop host1`
    And sleep 3
    When I successfully run `trema start host1`
    And sleep 10
    Then the file named "vhost.host1.pid" should exist

  @sudo
  Scenario: "Open vSwitch is already running" error
    When I run `trema start 0xabc`
    Then the exit status should not be 0
    And the output should contain:
      """
      error: Open vSwitch (dpid = 2748) is already running!
      """

  @sudo @wip
  Scenario: "vswitch is already running" error
    When I run `trema start host1`
    Then the exit status should not be 0
    And the output should contain:
      """
      error: vhost "host1" is already running!
      """

  @sudo
  Scenario: start NO_SUCH_NAME
    When I run `trema start NO_SUCH_NAME`
    Then the exit status should not be 0
    And the output should contain:
      """
      "NO_SUCH_NAME" does not exist.
      """
