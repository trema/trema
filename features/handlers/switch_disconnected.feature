Feature: switch_disconnected handler
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "switch_disconnected.rb" with:
      """ruby
      class SwitchDisconnected < Trema::Controller
        def switch_disconnected(dpid)
          logger.info "Switch #{dpid.to_hex} is disconnected."
        end
      end
      """
    And a file named "trema.conf" with:
      """ruby
      vswitch { datapath_id 0xabc }
      """

  @sudo
  Scenario: invoke switch_disconnected handler
    Given I use OpenFlow 1.0
    And I trema run "switch_disconnected.rb" with the configuration "trema.conf"
    When I successfully run `trema stop 0xabc`
    Then the file "SwitchDisconnected.log" should contain:
      """
      Switch 0xabc is disconnected.
      """

  @sudo
  Scenario: invoke switch_disconnected handler (OpenFlow 1.3)
    Given I use OpenFlow 1.3
    And I trema run "switch_disconnected.rb" with the configuration "trema.conf"
    When I successfully run `trema stop 0xabc`
    Then the file "SwitchDisconnected.log" should contain:
      """
      Switch 0xabc is disconnected.
      """
