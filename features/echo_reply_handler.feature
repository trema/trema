Feature: Trema::Controller#echo_reply handler
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |

  @sudo
  Scenario: invoke Trema::Controller#echo_reply handler
    Given a file named "echo_reply.rb" with:
      """
      class EchoReply < Trema::Controller
        def switch_ready(dpid)
          send_message dpid, Echo::Request.new
        end

        def echo_reply(dpid, message)
          logger.info 'echo_reply handler is invoked'
        end
      end
      """
    And a file named "trema.conf" with:
      """
      vswitch { datapath_id 0xabc }
      """
    When I successfully run `trema -v run echo_reply.rb -c trema.conf -d`
    And I run `sleep 5`
    Then the file "EchoReply.log" should contain "echo_reply handler is invoked"
