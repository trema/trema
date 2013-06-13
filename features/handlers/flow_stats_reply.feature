Feature: flow_stats_reply handlers

  The flow_stats_reply is a message handler to get information about flows in a switch.

  This handler can treat FlowStatsReply object through the 'message' parameter which is 
  the second argument of this handler. This object has configuration information
  (e.g. priority of this entry, timeout parameter, and the Match object which has been set 
  during installation, etc) and statistical information (e.g. the number of packets that this
  flow has processed). For more information about this, you can see in the Trema API document
  (http://rubydoc.info/github/trema/trema/master/Trema/FlowStatsReply).

  To handle this message handler, you should send an OpenFlow message which is named 
  Read-State message to the switch. The Read-State message is classified by 
  the type of information to get, and the type is identified by the 'type' parameter of 
  Read-State request message, but the Trema abstracts this mechanism.

  To send a Read-State request message that is corresponding to the flow_stats_reply,
  the controller should make that using FlowStatsRequest.new, and sends it using 
  send_message method as shown below. Detail of the parameters of this class is described 
  in the document (http://rubydoc.info/github/trema/trema/master/Trema/FlowStatsRequest).

  Scenario: flow_stats_reply handler
    Given a file named "flow-stats-reply-checker.rb" with:
    """
    class FlowStatsReplyChecker < Controller
      def switch_ready datapath_id
        # This is for getting a reply of ofp_flow_stats
        send_flow_mod_add datapath_id, :match => Match.new

        send_message datapath_id, FlowStatsRequest.new( :match => Match.new ) 
      end


      def flow_stats_reply datapath_id, message
        message.stats.each do | each |
          info "length : #{ each.length }"
          info "table_id : #{ each.table_id }"
          info "match : #{ each.match }"
          info "duration_sec : #{ each.duration_sec }"
          info "duration_nsec : #{ each.duration_nsec }"
          info "priority : #{ each.priority }"
          info "idle_timeout : #{ each.idle_timeout }"
          info "hard_timeout : #{ each.hard_timeout }"
          info "cookie : #{ each.cookie }"
          info "packet_count : #{ each.packet_count }"
          info "byte_count : #{ each.byte_count }"
          info "actions : #{ each.actions }"
        end
      end
    end
    """
    And a file named "sample.conf" with:
    """
    vswitch { datapath_id "0xabc" }
    """
    When I run `trema run ./flow-stats-reply-checker.rb -c sample.conf` interactively
    Then the output should contain "length : " within the timeout period
     And the output should contain "table_id : " within the timeout period
     And the output should contain "match : " within the timeout period
     And the output should contain "duration_sec : " within the timeout period
     And the output should contain "duration_nsec : " within the timeout period
     And the output should contain "priority : " within the timeout period
     And the output should contain "idle_timeout : " within the timeout period
     And the output should contain "hard_timeout : " within the timeout period
     And the output should contain "cookie : " within the timeout period
     And the output should contain "packet_count : " within the timeout period
     And the output should contain "byte_count : " within the timeout period
     And the output should contain "actions : " within the timeout period
