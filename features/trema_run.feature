Feature: trema run command
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |

  @sudo
  Scenario: the default port
    Given a file named "switch_ready.rb" with:
      """
      class SwitchReady < Trema::Controller
        def switch_ready(dpid)
          logger.info format('Hello %s!', dpid.to_hex)
        end
      end
      """
    And a file named "trema.conf" with:
      """
      vswitch { datapath_id 0xabc }
      """
    When I successfully run `trema -v run switch_ready.rb -c trema.conf -d`
    And I run `sleep 5`
    Then the file "SwitchReady.log" should contain "Hello 0xabc!"

  @sudo
  Scenario: -p option
    Given a file named "switch_ready.rb" with:
      """
      class SwitchReady < Trema::Controller
        def switch_ready(dpid)
          logger.info format('Hello %s!', dpid.to_hex)
        end
      end
      """
    And a file named "trema.conf" with:
      """
      vswitch {
        datapath_id 0xabc
        port 1234
      }
      """
    When I successfully run `trema -v run -p 1234 switch_ready.rb -c trema.conf -d`
    And I run `sleep 5`
    Then the file "SwitchReady.log" should contain "Hello 0xabc!"

