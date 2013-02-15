Feature: port_stats_reply handlers

  The port_stats_reply is a message handler to get statistics about physical ports of a switch.

  This handler can treat PortStatsReply object through the 'message' parameter which is 
  the second argument of this handler. This object has statistics about 
  each physical port of the switch (e.g. the number of bytes of packets sent and received 
  in the past, the number of packets dropped, and the number of packets error, etc). 
  For more information about this, you can see in the Trema API document 
  (http://rubydoc.info/github/trema/trema/master/Trema/PortStatsReply).

  To handle this message handler, you should send an OpenFlow message which is named 
  Read-State message to the switch. The Read-State message is classified by 
  a type of information to get, and the type is identified by the 'type' parameter of 
  Read-State request message, but this mechanism is abstracted by the Trema.

  To send a Read-State request message that is corresponding to the port_stats_reply,
  the controller should make that using PortStatsRequest.new, and sends it using 
  send_message method as shown below. Detail of the parameters of this class is described 
  in the document (http://rubydoc.info/github/trema/trema/master/Trema/PortStatsRequest).

  Scenario: port_stats_reply handler
    Given a file named "port-stats-reply-checker.rb" with:
    """
    class PortStatsReplyChecker < Controller
      def switch_ready datapath_id
        send_message datapath_id, PortStatsRequest.new 
      end


      def port_stats_reply datapath_id, message
        message.stats.each do | each |
          info "port_no : #{ each.port_no }"
          info "rx_packets : #{ each.rx_packets }"
          info "tx_packets : #{ each.tx_packets }"
          info "rx_bytes : #{ each.rx_bytes }"
          info "tx_bytes : #{ each.tx_bytes }"
          info "rx_dropped : #{ each.rx_dropped }"
          info "tx_dropped : #{ each.tx_dropped }"
          info "rx_errors : #{ each.rx_errors }"
          info "tx_errors : #{ each.tx_errors }"
          info "rx_frame_err : #{ each.rx_frame_err }"
          info "rx_over_err : #{ each.rx_over_err }"
          info "rx_crc_err : #{ each.rx_crc_err }"
          info "collisions : #{ each.collisions }"
        end
      end
    end
    """
    And a file named "sample.conf" with:
    """
    vswitch { datapath_id "0xabc" }
    """
    When I run `trema run ./port-stats-reply-checker.rb -c sample.conf` interactively
    Then the output should contain "port_no : 65534" within the timeout period
     And the output should contain "rx_packets : " within the timeout period
     And the output should contain "tx_packets : " within the timeout period
     And the output should contain "rx_bytes : " within the timeout period
     And the output should contain "tx_bytes : " within the timeout period
     And the output should contain "rx_dropped : " within the timeout period
     And the output should contain "tx_dropped : " within the timeout period
     And the output should contain "rx_errors : " within the timeout period
     And the output should contain "tx_errors : " within the timeout period
     And the output should contain "rx_frame_err : " within the timeout period
     And the output should contain "rx_over_err : " within the timeout period
     And the output should contain "rx_crc_err : " within the timeout period
     And the output should contain "collisions : " within the timeout period
