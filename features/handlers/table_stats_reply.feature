Feature: table_stats_reply handlers

  The Read-State message collects many kind of statistics from the switches.
  A kind of information to collect is determined by the type in request message.

  When controller sends a message of which message type is OFPST_TABLE, 
  table_stats_reply handler is invoked. This handler has object of TableStatsReply.
  And this object has information about table of switch.

  @wip
  Scenario: table_stats_reply handler
    Given a file named "table-stats-reply-checker.rb" with:
    """
    class TableStatsReplyChecker < Controller
      def switch_ready datapath_id
        send_message( datapath_id, TableStatsRequest.new )
      end

      def table_stats_reply datapath_id, message
        info "[ table_stats_reply ] message: #{ message.class }"
      end
    end
    """
    And a file named "sample.conf" with:
    """
    vswitch { datapath_id "0xabc" }
    """
    When I run `trema run ./table-stats-reply-checker.rb -c sample.conf` interactively
    Then the output should contain "[ table_stats_reply ]" within the timeout period
