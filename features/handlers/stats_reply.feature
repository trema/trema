Feature: x_stats_reply handlers

  The Read-State message collects many kind of statistics from the switches.
  A kind of information to collect is determined by the type in request message.
  To handle these reply messages associated with requests at your controller,
  you must implement :<type>_stats_reply method in the controller class.

  The all of :<type>_stats_reply handlers are specified as following in Trema.

  - desc_stats_reply       :  get information about switch manufacture
  - flow_stats_reply       :  get information about flows which are installed in switch.
  - aggregate_stats_reply  :  get aggregate statistics information about flows which are installed in switch
  - table_stats_reply      :  get statistics information about table of switch
  - port_stats_reply       :  get statistics information about physical ports of switch
  - queue_stats_reply      :  get statistics information about queue associated with ports.
  - vendor_stats_reply     :  get vendor specific information

  Background:
    Given a file named "stats_request.conf" with:
     """
     vswitch( "stats_request" ) { datapath_id "0xabc" }
     """

  @wip
  Scenario: Stats Request message in Ruby
    When I run `trema run ../../src/examples/openflow_message/stats-request.rb -c stats_request.conf -d`
     And wait until "StatsRequestController" is up
     And I run `trema killall`
    Then the file "../../tmp/log/StatsRequestController.log" should match /desc_stats_reply/
     And the file "../../tmp/log/StatsRequestController.log" should match /flow_stats_reply/
     And the file "../../tmp/log/StatsRequestController.log" should match /aggregate_stats_reply/
     And the file "../../tmp/log/StatsRequestController.log" should match /table_stats_reply/
     And the file "../../tmp/log/StatsRequestController.log" should match /port_stats_reply/


  @wip
  Scenario: Stats Request message in Ruby
    When I run `trema run ../../src/examples/openflow_message/obsolete-stats-request.rb -c stats_request.conf -d`
     And wait until "ObsoleteStatsRequest" is up
     And I run `trema killall`
    Then the file "../../tmp/log/ObsoleteStatsRequest.log" should match /Warning: 'stats_reply' handler will be deprecated/
     And the file "../../tmp/log/ObsoleteStatsRequest.log" should match /stats_reply/
     And the file "../../tmp/log/ObsoleteStatsRequest.log" should match /stats_reply/
     And the file "../../tmp/log/ObsoleteStatsRequest.log" should match /stats_reply/
     And the file "../../tmp/log/ObsoleteStatsRequest.log" should match /stats_reply/
     And the file "../../tmp/log/ObsoleteStatsRequest.log" should match /stats_reply/


  @wip
  Scenario: Stats Request message in Ruby
    When I run `trema run ../../src/examples/openflow_message/hybrid-stats-request.rb -c stats_request.conf -d`
     And wait until "HybridStatsRequest" is up
     And I run `trema killall`
    Then the file "../../tmp/log/HybridStatsRequest.log" should match /Warning: 'stats_reply' handler will be deprecated/
     And the file "../../tmp/log/HybridStatsRequest.log" should match /stats_reply/
     And the file "../../tmp/log/HybridStatsRequest.log" should match /stats_reply/
