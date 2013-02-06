Feature: port_stats_reply handlers

  The Read-State message collects many kind of statistics from the switches.
  A kind of information to collect is determined by the type in request message.

  When controller sends a message of which message type is OFPST_PORT, 
  port_stats_reply handler is invoked. This handler have objects of PortStatsReply.
  And these objects have information about physical port of switch.

  @wip
  Scenario: port_stats_reply handler
    Given a file named "port-stats-reply-checker.rb" with:
    """
    class PortStatsReplyChecker < Controller
      def switch_ready datapath_id
        send_message( datapath_id, PortStatsRequest.new )
      end

      def port_stats_reply datapath_id, message
        info "[ port_stats_reply ] message: #{ message.class }"
      end
    end
    """
    And a file named "sample.conf" with:
    """
    vswitch { datapath_id "0xabc" }
    """
    When I run `trema run ./port-stats-reply-checker.rb -c sample.conf` interactively
    Then the output should contain "[ port_stats_reply ]" within the timeout period

