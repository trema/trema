Feature: stats_reply handlers

  The Read-State message collects many kind of statistics from the switches.
  A kind of information to collect is determined by the type in request message.
  Through :stats_reply method you can handle all reply messages associated with requests 
  at your controller. But this callback method will be described in near future version.

  For backward compatibility, if controller defines both obsolete stats_reply handler and
  new type-specific handlers (port_stats_reply, flow_stats_reply, etc..), Trema fires former.

  @wip
  Scenario: obsolete stats_reply handler
    Given a file named "obsolete-stats-reply-checker.rb" with:
    """
    class ObsoleteStatsReplyChecker < Controller
      def switch_ready datapath_id
        # This is for getting a reply of ofp_flow_stats
        send_flow_mod_add( datapath_id, :match => Match.new)

        send_message( datapath_id, DescStatsRequest.new )
        send_message( datapath_id, FlowStatsRequest.new( :match => Match.new ) )
        send_message( datapath_id, AggregateStatsRequest.new( :match => Match.new ) )
        send_message( datapath_id, TableStatsRequest.new )
        send_message( datapath_id, PortStatsRequest.new )
      end

      def stats_reply datapath_id, message
        info "[ stats_reply ] message: #{ message.class }"
      end
    end
    """
    And a file named "sample.conf" with:
    """
    vswitch { datapath_id "0xabc" }
    """
    When I run `trema run ./obsolete-stats-reply-checker.rb -c sample.conf` interactively
    Then the output should contain "Warning: 'stats_reply' handler will be deprecated" within the timeout period
     And the output should contain "[ stats_reply ]" within the timeout period
     And the output should contain "[ stats_reply ]" within the timeout period
     And the output should contain "[ stats_reply ]" within the timeout period
     And the output should contain "[ stats_reply ]" within the timeout period
     And the output should contain "[ stats_reply ]" within the timeout period

  @wip
  Scenario: hybrid stats_reply handler
    Given a file named "hybrid-stats-reply-checker.rb" with:
    """
    class HybridStatsReplyChecker < Controller
      def switch_ready datapath_id
        send_message( datapath_id, TableStatsRequest.new )
        send_message( datapath_id, PortStatsRequest.new )
      end

      def port_stats_reply datapath_id, message
        info "[ port_stats_reply ] message: #{ message.class }"
      end
    
    
      def stats_reply datapath_id, message
        info "[ stats_reply ] message: #{ message.class }"
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
