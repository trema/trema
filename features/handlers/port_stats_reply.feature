Feature: port_stats_reply handlers

  The Read-State message collects many kind of statistics from the switches.
  A kind of information to collect is determined by the type in request message.

  When controller sends a message of which message type is OFPST_PORT, 
  port_stats_reply handler is invoked. This handler have objects of PortStatsReply.
  And these objects have information about physical port of switch.

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

    vhost "host1"
    vhost "host2"

    link "0xabc", "host1"
    link "0xabc", "host2"
    """
    When I run `trema run ./port-stats-reply-checker.rb -c sample.conf` interactively
    Then the output should contain "port_no : 1" within the timeout period
    Then the output should contain "port_no : 2" within the timeout period
    Then the output should contain "port_no : 65534" within the timeout period
