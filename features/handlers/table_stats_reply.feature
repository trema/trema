Feature: table_stats_reply handlers

  The table_stats_reply is a message handler to get information about tables that 
  a switch supports.

  This handler can treat TableStatsReply object through the 'message' parameter which is 
  the second argument of this handler. This object has configuration information, 
  table_id, name, the number of maximum flows that the switch can contain, etc. For more 
  information about this, you can see in the Trema API document 
  (http://rubydoc.info/github/trema/trema/master/Trema/TableStatsReply).

  To handle this message handler, you should send an OpenFlow message which is named 
  Read-State message to the switch. The Read-State message is classified by 
  the type of information to get, and the type is identified by the 'type' parameter of 
  Read-State request message, but the Trema abstracts this mechanism.

  To send a Read-State request message that is corresponding to the table_stats_reply,
  the controller should make that using TableStatsRequest.new, and send it using 
  send_message method as shown below. Detail of the parameters of this class is described 
  in the document (http://rubydoc.info/github/trema/trema/master/Trema/TableStatsRequest).

  Scenario: table_stats_reply handler
    Given a file named "table-stats-reply-checker.rb" with:
    """
    class TableStatsReplyChecker < Controller
      def switch_ready datapath_id
        send_message datapath_id, TableStatsRequest.new 
      end


      def table_stats_reply datapath_id, message
        message.stats.each do | each |
          info "table_id : #{ each.table_id }"
          info "name : #{ each.name }"
          info "wildcards : #{ each.wildcards }"
          info "max_entries : #{ each.max_entries }"
          info "active_count : #{ each.active_count }"
          info "lookup_count : #{ each.lookup_count }"
          info "matched_count : #{ each.matched_count }"
        end
      end
    end
    """
    And a file named "sample.conf" with:
    """
    vswitch { datapath_id "0xabc" }
    """
    When I run `trema run ./table-stats-reply-checker.rb -c sample.conf` interactively
    Then the output should contain "table_id : " within the timeout period
     And the output should contain "name : " within the timeout period
     And the output should contain "wildcards : " within the timeout period
     And the output should contain "max_entries : " within the timeout period
     And the output should contain "active_count : " within the timeout period
     And the output should contain "lookup_count : " within the timeout period
     And the output should contain "matched_count : " within the timeout period
