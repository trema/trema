Feature: "Vendor Stats Request" sample application

  In order to learn how to send Vendor Stats Request
  As a developer using Trema
  I want to execute "Vendor Stats Request" sample application

  Background:
    Given a file named "vendor_stats.conf" with:
      """
      vswitch( "vendor_stats" ) { datapath_id "0xabc" }
      """

  @slow_process
  Scenario: Vendor Stats Request message in Ruby
  Given I run `trema run ../../src/examples/openflow_message/vendor-stats-request.rb -c vendor_stats.conf -d`
   And wait until "VendorStatsRequestSample" is up
   And *** sleep 2 ***
  Then the file "../../tmp/log/openflowd.vendor_stats.log" should contain "received: NXST_FLOW reques"
   And the file "../../tmp/log/VendorStatsRequestSample.log" should contain "[vendor_stats_reply]"
   And the file "../../tmp/log/VendorStatsRequestSample.log" should contain "vendor_id: 0x00002320"
