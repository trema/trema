Feature: "Vendor Action" sample application

  In order to learn how to send Vendor Action
  As a developer using Trema
  I want to execute "Vendor Action" sample application

  Background:
    Given a file named "vendor_action.conf" with:
      """
      vswitch( "vendor_action" ) { datapath_id "0xabc" }
      """

  @slow_process
  Scenario: Vendor Action message in C
  Given I run `trema run ../../objects/examples/openflow_message/vendor_action -c vendor_action.conf -d`
   And wait until "vendor_action" is up
   And *** sleep 2 ***
  Then the file "../../tmp/log/openflowd.vendor_action.log" should contain "actions=note:54.72.65.6d.61.00"

  @slow_process
  Scenario: Vendor Action message in Ruby
  Given I run `trema run ../../src/examples/openflow_message/vendor-action.rb -c vendor_action.conf -d`
   And wait until "VendorActionSampleController" is up
   And *** sleep 2 ***
  Then the file "../../tmp/log/openflowd.vendor_action.log" should contain "actions=note:54.72.65.6d.61.00"
