Feature: desc_stats_reply handlers

  The Read-State message collects many kind of statistics from the switches.
  A kind of information to collect is determined by the type in request message.

  When controller sends a message of which message type is OFPST_DESC, 
  desc_stats_reply handler is invoked. This handler has an object of DescStatsReply.
  And this object has information about switch manufacture.

  Scenario: desc_stats_reply handler
    Given a file named "desc-stats-reply-checker.rb" with:
    """
    class DescStatsReplyChecker < Controller
      def switch_ready datapath_id
        send_message datapath_id, DescStatsRequest.new 
      end


      def desc_stats_reply datapath_id, message
        reply = message.stats[0]
    
        info "mfr_desc : #{ reply.mfr_desc }"
        info "hw_desc : #{ reply.hw_desc }"
        info "sw_desc : #{ reply.sw_desc }"
        info "serial_num : #{ reply.serial_num }"
        info "dp_desc : #{ reply.dp_desc }"
      end
    end
    """
    And a file named "sample.conf" with:
    """
    vswitch { datapath_id "0xabc" }
    """
    When I run `trema run ./desc-stats-reply-checker.rb -c sample.conf` interactively
    Then the output should contain "mfr_desc : Nicira Networks, Inc." within the timeout period
    Then the output should contain "hw_desc : Open vSwitch" within the timeout period
