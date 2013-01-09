Feature: Send a features request message

  In order to get the list of switch features
  As a Trema user
  I want to send a features request message to openflow switches

  Background:
    Given a file named "features_request.conf" with:
     """
     vswitch( "features_request" ) { datapath_id "0xabc" }
     """

  @slow_process
  Scenario: Features Request message in C
    When I run `trema run ../../objects/examples/openflow_message/features_request -c features_request.conf -d`
     And wait until "features_request" is up
     And I run `trema killall`
    Then the file "../../tmp/log/features_request.log" should match /datapath_id: 0xabc/
     And the file "../../tmp/log/features_request.log" should match /n_buffers:/
     And the file "../../tmp/log/features_request.log" should match /n_tables:/
     And the file "../../tmp/log/features_request.log" should match /capabilities:/
     And the file "../../tmp/log/features_request.log" should match /actions:/
     And the file "../../tmp/log/features_request.log" should match /ports:/

  @slow_process
  Scenario: Feature Request message in Ruby
    When I run `trema run ../../src/examples/openflow_message/features-request.rb -c features_request.conf -d`
     And wait until "FeaturesRequestController" is up
     And I run `trema killall`
    Then the file "../../tmp/log/FeaturesRequestController.log" should match /datapath_id: 0xabc/
     And the file "../../tmp/log/FeaturesRequestController.log" should match /n_buffers:/
     And the file "../../tmp/log/FeaturesRequestController.log" should match /n_tables:/
     And the file "../../tmp/log/FeaturesRequestController.log" should match /capabilities:/
     And the file "../../tmp/log/FeaturesRequestController.log" should match /actions:/
     And the file "../../tmp/log/FeaturesRequestController.log" should match /ports:/
