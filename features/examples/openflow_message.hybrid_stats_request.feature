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
    When I run `trema run ../../src/examples/openflow_message/hybrid-stats-request.rb -c stats_request.conf -d`
     And wait until "HybridStatsRequest" is up
     And I run `trema killall`
    Then the file "../../tmp/log/HybridStatsRequest.log" should match /stats_reply/
     And the file "../../tmp/log/HybridStatsRequest.log" should match /port_stats_reply/
