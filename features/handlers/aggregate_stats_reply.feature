Feature: aggregate_stats_reply handlers

  The Read-State message collects many kind of statistics from the switches.
  A kind of information to collect is determined by the type in request message.

  When controller sends a message of which message type is OFPST_AGGREGATE, 
  aggregate_stats_reply handler is invoked. This handler has an object of AggregateStatsReply.
  And this object has aggregate statistical information of flows installed in switch.

  Scenario: aggregate_stats_reply handler
    Given a file named "aggregate-stats-reply-checker.rb" with:
    """
    class AggregateStatsReplyChecker < Controller
      def switch_ready datapath_id
        send_flow_mod_add datapath_id, :match => Match.new

        send_message datapath_id, AggregateStatsRequest.new( :match => Match.new )
      end


      def aggregate_stats_reply datapath_id, message
        reply = message.stats[0]
    
        info "packet_count : #{ reply.packet_count }"
        info "byte_count : #{ reply.byte_count }"
        info "flow_count : #{ reply.flow_count }"
      end
    end
    """
    And a file named "sample.conf" with:
    """
    vswitch { datapath_id "0xabc" }
    """
    When I run `trema run ./aggregate-stats-reply-checker.rb -c sample.conf` interactively
    Then the output should contain "flow_count : 1" within the timeout period
