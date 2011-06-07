Feature: "Getting switch info" sample application

  As a Trema user
  I want to write "Getting switch info" application
  So that I can learn how to acquire the information of openflow switches


  Scenario: Getting switch info in C
    When I try trema run "./objects/examples/switch_info/switch_info" with following configuration:
      """
      vswitch { datapath_id "0xabc" }
      """
    Then the output should include:
      """
      datapath_id: 0xabc
      #ports: 1
      """


  Scenario: Getting switch info in C
    When I try trema run "./objects/examples/switch_info/switch_info" with following configuration:
      """
      vswitch { datapath_id "0xabc" }
      vswitch { datapath_id "0xdef" }
      link "0xabc", "0xdef"
      """
    Then the output should include:
      """
      #ports: 2
      """


  Scenario: Getting switch info in Ruby
    When I try trema run "./src/examples/switch_info/switch_info.rb" with following configuration:
      """
      vswitch { datapath_id "0xabc" }
      """
    Then the output should include:
      """
      datapath_id: 0xabc
      ports: 65534
      """


  Scenario: Getting switch info in Ruby
    When I try trema run "./src/examples/switch_info/switch_info.rb" with following configuration:
      """
      vswitch { datapath_id "0xabc" }
      vswitch { datapath_id "0xdef" }
      link "0xabc", "0xdef"
      """
    Then the output should include:
      """
      ports: 1, 65534
      """
