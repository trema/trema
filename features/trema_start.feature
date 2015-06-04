Feature: trema start command
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "switch_ready_controller.rb" with:
      """
      class SwitchReadyController < Trema::Controller
        def switch_ready(dpid)
          if @reconnect
            logger.info "Switch #{dpid.to_hex} connected again."
          else
            logger.info "Switch #{dpid.to_hex} connected."
            @reconnect = true
          end
        end
      end
      """
    And a file named "trema.conf" with:
      """
      vswitch { datapath_id 0xabc }
      vhost('host1') { ip '192.168.0.1' }
      link '0xabc', 'host1'
      """
    And I run `trema run switch_ready_controller.rb -c trema.conf -d`

  @sudo
  Scenario: trema stop and start switch_name
    Given I successfully run `trema stop 0xabc`
    When I successfully run `trema start 0xabc`
    And I successfully run `sleep 10`
    Then the file "SwitchReadyController.log" should contain:
      """
      Switch 0xabc connected again.
      """

  @sudo
  Scenario: trema stop and start host_name
    Given I successfully run `trema stop host1`
    And I successfully run `sleep 3`
    When I successfully run `trema start host1`
    And I successfully run `sleep 10`
    Then the following files should exist:
      | vhost.host1.pid |

  @sudo
  Scenario: start switch_name (already running)
    When I run `trema start 0xabc`
    Then the exit status should not be 0
    And the output should contain:
      """
      error: Open vSwitch (dpid = 2748) is already running!
      """

  @sudo
  Scenario: start host_name (already running)
    When I run `trema start host1`
    Then the exit status should not be 0
    # And the output should contain:
    #   """
    #   error: vhost "host1" is already running!
    #   """

  @sudo
  Scenario: start NO_SUCH_NAME
    When I run `trema start NO_SUCH_NAME`
    Then the exit status should not be 0
    And the output should contain:
      """
      "NO_SUCH_NAME" does not exist.
      """
