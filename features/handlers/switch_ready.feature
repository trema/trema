Feature: switch_ready handler
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "switch_ready.rb" with:
      """ruby
      class SwitchReady < Trema::Controller
        def switch_ready(dpid)
          logger.info format('Hello %s!', dpid.to_hex)
        end
      end
      """
    And a file named "trema.conf" with:
      """ruby
      vswitch { datapath_id 0xabc }
      """

  @sudo
  Scenario: invoke switch_ready handler
    Given I use OpenFlow 1.0
    When I trema run "switch_ready.rb" with the configuration "trema.conf"
    Then the file "SwitchReady.log" should contain "Hello 0xabc!"

  @sudo
  Scenario: invoke switch_ready handler (OpenFlow 1.3)
    Given I use OpenFlow 1.3
    When I trema run "switch_ready.rb" with the configuration "trema.conf"
    Then the file "SwitchReady.log" should contain "Hello 0xabc!"
