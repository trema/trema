Feature: stats_reply handlers

  The stats_reply is a message handler to get any kind of statistical information, but note
  that this handler will be deprecated in near future. Instead, you should use type-specific 
  handlers (desc_stats_reply, flow_stats_reply, aggregate_stats_reply, table_stats_reply,
  port_stats_reply, queue_stats_reply and vendor_stats_reply).

  This handler can treat StatsReply object through the 'message' parameter which is the second 
  argument of this handler. This object has the reply message corresponding to the request.
  For more information about this, you can see in the Trema API document 
  (http://rubydoc.info/github/trema/trema/master/Trema/StatsReply).

  To handle this message handler, you should send an OpenFlow message which is named 
  Read-State message to the switch. The Read-State message is classified by 
  the type of information to get, and the type is identified by the 'type' parameter of 
  Read-State request message, but the Trema abstracts this mechanism.

  To send a Read-State request message, the controller should instantiate a sub-class 
  of StatsRequest (DescStatsRequest, FlowStatsRequest, AggregateStatsRequest, TableStatsRequest, 
  PortStatsRequest, QueueStatsRequest and VendorStatsRequest), and sends it using send_message 
  method as shown below.

  For backward compatibility, if the controller defines both obsolete stats_reply handler and
  new type-specific handlers (port_stats_reply, flow_stats_reply, etc..), Trema fires former.

  Scenario: obsolete stats_reply handler
    Given a file named "obsolete-stats-reply-checker.rb" with:
    """
    class ObsoleteStatsReplyChecker < Controller
      def switch_ready datapath_id
        # This is for getting a reply of ofp_flow_stats
        send_flow_mod_add datapath_id, :match => Match.new
    
        send_message datapath_id, DescStatsRequest.new 
        send_message datapath_id, FlowStatsRequest.new( :match => Match.new ) 
        send_message datapath_id, AggregateStatsRequest.new( :match => Match.new ) 
        send_message datapath_id, TableStatsRequest.new 
        send_message datapath_id, PortStatsRequest.new 
      end


      def stats_reply datapath_id, message
        message.stats.each do | each |
          info "message : #{ each.class }"
        end
      end
    end
    """
    And a file named "sample.conf" with:
    """
    vswitch { datapath_id "0xabc" }
    """
    When I run `trema run ./obsolete-stats-reply-checker.rb -c sample.conf` interactively
    Then the output should contain "Warning: 'stats_reply' handler will be deprecated" within the timeout period
      And the output should contain "message : Trema::DescStatsReply" within the timeout period
      And the output should contain "message : Trema::FlowStatsReply" within the timeout period
      And the output should contain "message : Trema::AggregateStatsReply" within the timeout period
      And the output should contain "message : Trema::TableStatsReply" within the timeout period
      And the output should contain "message : Trema::PortStatsReply" within the timeout period

  Scenario: hybrid stats_reply handler
    Given a file named "hybrid-stats-reply-checker.rb" with:
    """
    class HybridStatsReplyChecker < Controller
      def switch_ready datapath_id
        send_message datapath_id, TableStatsRequest.new 
        send_message datapath_id, PortStatsRequest.new 
      end


      def port_stats_reply datapath_id, message
        info "[ port_stats_reply ]"
      end
    
    
      def stats_reply datapath_id, message
        info "[ stats_reply ]"
      end
    end
    """
    And a file named "sample.conf" with:
    """
    vswitch { datapath_id "0xabc" }
    """
    When I run `trema run ./hybrid-stats-reply-checker.rb -c sample.conf` interactively
    Then the output should contain "Warning: 'stats_reply' handler will be deprecated" within the timeout period
     And the output should contain "[ stats_reply ]" within the timeout period
     And the output should contain "[ stats_reply ]" within the timeout period
