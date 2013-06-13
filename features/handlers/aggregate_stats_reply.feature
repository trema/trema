Feature: aggregate_stats_reply handlers

  The aggregate_stats_reply is a message handler to get aggregate statistics of flows in a switch.

  This handler can treat AggregateStatsReply object through the 'message' parameter which is 
  the second argument of this handler. This object has statistics about aggregate number of bytes, 
  number of packets, and flow counts. For more information about this, you can see in the Trema API 
  document (http://rubydoc.info/github/trema/trema/master/Trema/AggregateStatsReply).

  To handle this message handler, you should send an OpenFlow message which is named 
  Read-State message to the switch. The Read-State message is classified by 
  the type of information to get, and the type is identified by the 'type' parameter of 
  Read-State request message, but the Trema abstracts this mechanism.

  To send a Read-State request message that is corresponding to the aggregate_stats_reply,
  the controller should make that using AggregateStatsRequest.new with Match object that is a 
  configuration which flows to get, and sends it using send_message method as shown below. 
  Detail of the parameters of this class is described in the document 
  (http://rubydoc.info/github/trema/trema/master/Trema/AggregateStatsRequest).

  Scenario: aggregate_stats_reply handler
    Given a file named "aggregate-stats-reply-checker.rb" with:
    """
    class AggregateStatsReplyChecker < Controller
      def switch_ready datapath_id
        send_flow_mod_add datapath_id, :match => Match.new

        send_message datapath_id, AggregateStatsRequest.new( :match => Match.new )
      end


      def aggregate_stats_reply datapath_id, message
        message.stats.each do | each |
          info "packet_count : #{ each.packet_count }"
          info "byte_count : #{ each.byte_count }"
          info "flow_count : #{ each.flow_count }"
        end
      end
    end
    """
    And a file named "sample.conf" with:
    """
    vswitch { datapath_id "0xabc" }
    """
    When I run `trema run ./aggregate-stats-reply-checker.rb -c sample.conf` interactively
    Then the output should contain "packet_count : " within the timeout period
     And the output should contain "byte_count : " within the timeout period
     And the output should contain "flow_count : " within the timeout period
