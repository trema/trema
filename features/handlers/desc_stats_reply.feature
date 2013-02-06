Feature: desc_stats_reply handlers

  The Read-State message collects many kind of statistics from the switches.
  A kind of information to collect is determined by the type in request message.

  When controller sends a message of which message type is OFPST_DESC, 
  desc_stats_reply handler is invoked. This handler has an object of DescStatsReply.
  And this object has information about switch manufacture.

  @wip
  Scenario: desc_stats_reply handler
    Given a file named "desc-stats-reply-checker.rb" with:
    """
    class DescStatsReplyChecker < Controller
      def switch_ready datapath_id
        send_message( datapath_id, DescStatsRequest.new )
      end

      def desc_stats_reply datapath_id, message
        info "[ desc_stats_reply ] message: #{ message.class }"
      end
    end
    """
    And a file named "sample.conf" with:
    """
    vswitch { datapath_id "0xabc" }
    """
    When I run `trema run ./desc-stats-reply-checker.rb -c sample.conf` interactively
    Then the output should contain "[ desc_stats_reply ]" within the timeout period
