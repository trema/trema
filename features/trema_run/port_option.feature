Feature: -p (--port) option

  -p (--port) option overrides the default openflow channel port.

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
          logger.info 'connected to port 1234'
        end
      end
      """
    And a file named "trema.conf" with:
      """ruby
      vswitch {
        datapath_id 0xabc
        port 1234
      }
      """

  @sudo
  Scenario: -p option
    When I successfully run `trema run -p 1234 switch_ready.rb -c trema.conf -d`
    And sleep 5
    Then the file "SwitchReady.log" should contain "connected to port 1234"

  @sudo
  Scenario: --port option
    When I successfully run `trema run --port 1234 switch_ready.rb -c trema.conf -d`
    And sleep 5
    Then the file "SwitchReady.log" should contain "connected to port 1234"
