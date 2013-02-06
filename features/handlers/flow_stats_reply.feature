Feature: flow_stats_reply handlers

  The Read-State message collects many kind of statistics from the switches.
  A kind of information to collect is determined by the type in request message.

  When controller sends a message of which message type is OFPST_FLOW, 
  flow_stats_reply handler is invoked. This handler has objects of FlowStatsReply.
  And these objects have statistical information about flow installed in switch.

  @wip
  Scenario: flow_stats_reply handler
    Given a file named "flow-stats-reply-checker.rb" with:
    """
    class FlowStatsReplyChecker < Controller
      def switch_ready datapath_id
        # This is for getting a reply of ofp_flow_stats
        send_flow_mod_add( datapath_id, :match => Match.new )

        send_message( datapath_id, FlowStatsRequest.new( :match => Match.new ) )
      end

      def flow_stats_reply datapath_id, message
        info "[ flow_stats_reply ] message: #{ message.class }"
      end
    end
    """
    And a file named "sample.conf" with:
    """
    vswitch { datapath_id "0xabc" }
    """
    When I run `trema run ./flow-stats-reply-checker.rb -c sample.conf` interactively
    Then the output should contain "[ flow_stats_reply ]" within the timeout period
