Feature: stop
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "switch_disconnected_controller.rb" with:
      """ruby
      class SwitchDisconnectedController < Trema::Controller
        def switch_disconnected(dpid)
          logger.info "Switch #{dpid.to_hex} is disconnected."
        end
      end
      """
    And a file named "trema.conf" with:
      """ruby
      vswitch { datapath_id 0xabc }

      vhost('host1') { ip '192.168.0.1' }
      vhost('host2') { ip '192.168.0.2' }

      link '0xabc', 'host1'
      link '0xabc', 'host2'
      """
    And I trema run "switch_disconnected_controller.rb" with the configuration "trema.conf"

  @sudo
  Scenario: stop a switch
    When I successfully run `trema stop 0xabc`
    And sleep 10
    Then the file "SwitchDisconnectedController.log" should contain:
      """
      Switch 0xabc is disconnected.
      """

  @sudo
  Scenario: stop a host
    When I successfully run `trema stop host1`
    And sleep 5
    Then the file "vhost.host1.pid" should not exist

  @sudo
  Scenario: stop NO_SUCH_NAME
    When I run `trema stop NO_SUCH_NAME`
    Then the exit status should not be 0
    And the output should contain:
    """
    "NO_SUCH_NAME" does not exist.
    """
