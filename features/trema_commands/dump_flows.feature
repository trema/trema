Feature: dump_flows command

  In order to inspect all flow entries in a Trema virtual switch
  As a developer using Trema
  I want to execute "trema dump_flows" command

  Background:
    Given a file named "repeater_hub.conf" with:
      """
      vswitch("repeater_hub") { datapath_id 0xabc }

      vhost("host1")
      vhost("host2")
      vhost("host3")

      link "repeater_hub", "host1"
      link "repeater_hub", "host2"
      link "repeater_hub", "host3"
      """
    And I successfully run `trema run ../../objects/examples/repeater_hub/repeater_hub -c repeater_hub.conf -d`

  @slow_process
  Scenario: dump a flow entry
    Given I run `trema send_packets --source host1 --dest host2`
    And *** sleep 1 ***
    When I run `trema dump_flows repeater_hub`
    Then the output should contain "actions=FLOOD"

  @slow_process
  Scenario: no flow entry
    When I run `trema dump_flows repeater_hub`
    Then the output should not contain "actions="

  Scenario: no argument
    When I run `trema dump_flows`
    Then the output should contain "switches is required"

  Scenario: wrong switch name
    When I run `trema dump_flows nosuchswitch`
    Then the output should contain "No switch named `nosuchswitch` found!"
