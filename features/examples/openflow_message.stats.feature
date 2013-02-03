Feature: Send stats messages

  In order to collects some statistics from openflow switches
  As a Trema user
  I want to send stats request messages to an openflow switch

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
