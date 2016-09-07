Feature: run
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |

  @sudo
  Scenario: run controller_file
    Given I use a fixture named "event_logger"
    When I trema run "event_logger.rb" with args "-d"
    Then the file "EventLogger.log" should contain:
      """
      EventLogger#start (args = [])
      """

  @sudo
  Scenario: cleanup on failure
    Given a file named "switch_ready_fail.rb" with:
      """ruby
      class SwitchReadyFail < Trema::Controller
        def switch_ready(_dpid)
          fail 'bang!'
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
    When I run `trema -v run switch_ready_fail.rb -c trema.conf`
    Then virtual links should not exist 
    And the following files should not exist:
      | SwitchReadyFail.pid    |
      | vhost.host1.pid        |
      | vhost.host2.pid        |
