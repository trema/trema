Feature: "set_forward_entries" C API example command 
  
  Switch Event forwarding configuration command (`set_forward_entries`)
  is a command to replace event forwarding entries of 
  Switch Manager and Switch Daemons.
  
  The types of switch event can be forwarded are:
  * vendor
  * packet_in
  * port_stat
  * state_notify
  
  This command is a simple usage example for event_forward_interface.h C API.
  The event_forward_interface.h API is used in topology manager to 
  add itself to packet_in forwarding entry of all existing switch daemons and 
  switch manager to receive LLDP packets.
  By removing entry for 'topology' from some switches, it is possible to make 
  topology manager to map only subset of all the switches managed by trema.
  
  Please see README.md for general notes on switch event forwarding APIs.

  Background: 
    Given I cd to "../../src/examples/switch_event_config/"
    Given I compile "set_forward_entries.c" into "set_forward_entries"
    Given I compile "dump_forward_entries.c" into "dump_forward_entries"

  Scenario: set_forward_entries Usage
    When I run `trema run './set_forward_entries -h'`
    Then the output should contain:
      """
      Set OpenFlow Switch Manager/Daemon event forward entries.
       Switch Manager: set_forward_entries -m -t EVENT_TYPE service_name1,service_name2,...
       Switch Daemon : set_forward_entries -s SWITCH_DPID -t EVENT_TYPE service_name1,service_name2,...
      
       EVENT_TYPE:
        -t, --type={vendor,packet_in,port_status,state_notify} Specify event type.
      """

  Scenario Outline: Replace Switch Manager's event forward configuration to 'mirror' and 'filter' for packet_in
    Given a file named "nw_dsl.conf" with:
      """
        vswitch { datapath_id 0x1 }
        vswitch { datapath_id 0x2 }
      """
    And I run `trema run ../repeater_hub/repeater-hub.rb -c nw_dsl.conf -d`
    And wait until "RepeaterHub" is up
    When I run `trema run './set_forward_entries -m -t packet_in mirror,filter'`
    Then the output should contain "Updated service name list:"
    Then the output should contain "  mirror"
    Then the output should contain "  filter"
    Then the output should not contain "  RepeaterHub"
    Then I run `trema run './dump_forward_entries -s <switch> -t packet_in'`
    Then the output should match /Current service name list:.*  RepeaterHub/
    Then the output should not match /Current service name list:.*  mirror/
    Then the output should not match /Current service name list:.*  filter/

    Examples: 
      | switch |
      | 0x1    |
      | 0x2    |

  Scenario Outline: Replace Switch Daemon 0x1's event forward configuration to 'mirror' and 'filter' for packet_in
    Given a file named "nw_dsl.conf" with:
      """
        vswitch { datapath_id 0x1 }
        vswitch { datapath_id 0x2 }
      """
    And I run `trema run ../repeater_hub/repeater-hub.rb -c nw_dsl.conf -d`
    And wait until "RepeaterHub" is up
    When I run `trema run './set_forward_entries -s 0x1 -t packet_in mirror,filter'`
    Then the output should contain "Updated service name list:"
    Then the output should contain "  mirror"
    Then the output should contain "  filter"
    Then the output should not contain "  RepeaterHub"
    Then I run `trema run './dump_forward_entries <option> -t packet_in'`
    Then the output should match /Current service name list:.*  RepeaterHub/
    Then the output should not match /Current service name list:.*  mirror.*/
    Then the output should not match /Current service name list:.*  filter.*/

    Examples: 
      | target | option |
      | SW MGR | -m     |
      | SW 0x2 | -s 0x2 |
