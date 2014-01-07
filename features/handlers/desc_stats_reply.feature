Feature: desc_stats_reply handlers

  The desc_stats_reply is a message handler to get informatoin about a switch manufacturer.

  This handler can treat DescStatsReply object through the 'message' parameter which is 
  the second argument of this handler. This object has information about the switch manufacturer, 
  hardware revision, software revision, serial number, and a description field is available. 
  For more information about this, you can see in the Trema API document 
  (http://rubydoc.info/github/trema/trema/master/Trema/DescStatsReply).

  To handle this message handler, you should send an OpenFlow message which is named 
  Read-State message to the switch. The Read-State message is classified by 
  the type of information to get, and the type is identified by the 'type' parameter of 
  Read-State request message, but the Trema abstracts this mechanism.

  To send a Read-State request message that is corresponding to the desc_stats_reply,
  the controller should make that using DescStatsRequest.new, and sends it using 
  send_message method as shown below. Detail of the parameters of this class is described 
  in the document (http://rubydoc.info/github/trema/trema/master/Trema/DescStatsRequest).

  Scenario: desc_stats_reply handler
    Given a file named "desc-stats-reply-checker.rb" with:
    """
    class DescStatsReplyChecker < Controller
      def switch_ready datapath_id
        send_message datapath_id, DescStatsRequest.new 
      end


      def desc_stats_reply datapath_id, message
        message.stats.each do | each |
          info "mfr_desc : #{ each.mfr_desc }"
          info "hw_desc : #{ each.hw_desc }"
          info "sw_desc : #{ each.sw_desc }"
          info "serial_num : #{ each.serial_num }"
          info "dp_desc : #{ each.dp_desc }"
        end
      end
    end
    """
    And a file named "sample.conf" with:
    """
    vswitch { datapath_id "0xabc" }
    """
    When I run `trema run ./desc-stats-reply-checker.rb -c sample.conf` interactively
    Then the output should contain "mfr_desc : " within the timeout period
     And the output should contain "hw_desc : " within the timeout period
     And the output should contain "sw_desc : " within the timeout period
     And the output should contain "serial_num : " within the timeout period
     And the output should contain "dp_desc : " within the timeout period
