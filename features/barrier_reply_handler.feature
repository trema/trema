Feature: Trema::Controller#barrier_reply handler
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |

  @sudo
  Scenario: invoke Trema::Controller#barrier_reply handler
    Given a file named "barrier_reply.rb" with:
      """
      class BarrierReply < Trema::Controller
        def switch_ready(dpid)
          send_message dpid, Barrier::Request.new
        end

        def barrier_reply(dpid, message)
          logger.info 'barrier_reply handler is invoked'
        end
      end
      """
    And a file named "trema.conf" with:
      """
      vswitch { datapath_id 0xabc }
      """
    When I successfully run `trema -v run barrier_reply.rb -c trema.conf -d`
    And I run `sleep 5`
    Then the file "BarrierReply.log" should contain "barrier_reply handler is invoked"
