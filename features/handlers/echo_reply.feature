Feature: echo_reply handler
  @sudo
  Scenario: invoke echo_reply handler
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
