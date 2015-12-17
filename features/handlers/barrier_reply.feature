Feature: barrier_reply handler
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "barrier_reply.rb" with:
      """ruby
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
      """ruby
      vswitch { datapath_id 0xabc }
      """

  @sudo
  Scenario: invoke barrier_reply handler
    Given I use OpenFlow 1.0
    When I trema run "barrier_reply.rb" with the configuration "trema.conf"
    Then the file "BarrierReply.log" should contain "barrier_reply handler is invoked"

  @sudo @wip
  Scenario: invoke barrier_reply handler
    Given I use OpenFlow 1.3
    When I trema run "barrier_reply.rb" with the configuration "trema.conf"
    Then the file "BarrierReply.log" should contain "barrier_reply handler is invoked"
