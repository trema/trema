Feature: echo_reply handler
  Background:
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

  @sudo
  Scenario: invoke echo_reply handler
    Given I use OpenFlow 1.0
    When I trema run "echo_reply.rb" with the configuration "trema.conf"
    Then the file "EchoReply.log" should contain "echo_reply handler is invoked"

  @sudo
  Scenario: invoke echo_reply handler (OpenFlow 1.3)
    Given I use OpenFlow 1.3
    When I trema run "echo_reply.rb" with the configuration "trema.conf"
    Then the file "EchoReply.log" should contain "echo_reply handler is invoked"
