Feature: description_stats_reply handler
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "description_stats_reply.rb" with:
      """ruby
      class DescriptionStatsReply < Trema::Controller
        def switch_ready(dpid)
          send_message dpid, DescriptionStats::Request.new
        end

        def description_stats_reply(dpid, message)
          logger.info 'description_stats_reply handler is invoked'
        end
      end
      """
    And a file named "trema.conf" with:
      """ruby
      vswitch { datapath_id 0xabc }
      """

  @sudo
  Scenario: invoke description_reply handler
    Given I use OpenFlow 1.0
    When I trema run "description_stats_reply.rb" with the configuration "trema.conf"
    Then the file "DescriptionStatsReply.log" should contain "description_stats_reply handler is invoked"

  @sudo @wip
  Scenario: invoke barrier_reply handler
    Given I use OpenFlow 1.3
    When I trema run "barrier_reply.rb" with the configuration "trema.conf"
    Then the file "BarrierReply.log" should contain "barrier_reply handler is invoked"
