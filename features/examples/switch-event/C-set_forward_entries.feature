Feature: C function example command "set_forward_entries"
  
  Switch Event forwarding configuration command (`set_forward_entries`)
  is a command to replace event forwarding entries of 
  Switch Manager and Switch Daemons.
  
  Following switch event types can be configured by this command:
  
  * vendor
  * packet_in
  * port_stat
  * state_notify
  
  This command is a simple usage example for C version of switch event forwarding API.
  C version API is defined as a group of functions declared in event_forward_interface.h.
  These functions are used in topology manager to 
  add itself to packet_in forwarding entry of all existing switch daemons and 
  switch manager to receive LLDP packets.
  By adding and removing entry for 'topology' from some switches, it is possible to make 
  topology manager to map only a subset of the switches managed by trema.
  
  Please see README.md for general notes on switch event forwarding API.

  Background: 
    Given I cd to "../../src/examples/switch_event_config/"
    And I compile "set_forward_entries.c" into "set_forward_entries"
    And I compile "dump_forward_entries.c" into "dump_forward_entries"

  Scenario: set_forward_entries Usage
    When I successfully run `trema run './set_forward_entries -h'`
    Then the output should contain:
      """
      Set OpenFlow Switch Manager/Daemon event forward entries.
       Switch Manager: set_forward_entries -m -t EVENT_TYPE service_name1,service_name2,...
       Switch Daemon : set_forward_entries -s SWITCH_DPID -t EVENT_TYPE service_name1,service_name2,...
      
       EVENT_TYPE:
        -t, --type={vendor,packet_in,port_status,state_notify} Specify event type.
      """

  Scenario Outline: Replace Switch Manager's event forwarding entries of packet_in to 'mirror' and 'filter'
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      vswitch { datapath_id 0x2 }
      """
    And I successfully run `trema run ../repeater_hub/repeater-hub.rb -c nw_dsl.conf -d`
    And wait until "RepeaterHub" is up
    When I successfully run `trema run './set_forward_entries -m -t packet_in mirror,filter'`
    Then the output should contain "Updated service name list:"
    And the output should contain "  mirror"
    And the output should contain "  filter"
    And the output should not contain "  RepeaterHub"
    And I successfully run `trema run './dump_forward_entries -s <switch> -t packet_in'`
    And the output should match /Current service name list:.*  RepeaterHub/
    And the output should not match /Current service name list:.*  mirror/
    And the output should not match /Current service name list:.*  filter/

    Examples: 
      | switch |
      | 0x1    |
      | 0x2    |

  Scenario Outline: Replace Switch Daemon 0x1's event forwarding entries of packet_in to 'mirror' and 'filter'
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      vswitch { datapath_id 0x2 }
      """
    And I successfully run `trema run ../repeater_hub/repeater-hub.rb -c nw_dsl.conf -d`
    And wait until "RepeaterHub" is up
    When I successfully run `trema run './set_forward_entries -s 0x1 -t packet_in mirror,filter'`
    Then the output should contain "Updated service name list:"
    And the output should contain "  mirror"
    And the output should contain "  filter"
    And the output should not contain "  RepeaterHub"
    And I successfully run `trema run './dump_forward_entries <option> -t packet_in'`
    And the output should match /Current service name list:.*  RepeaterHub/
    And the output should not match /Current service name list:.*  mirror.*/
    And the output should not match /Current service name list:.*  filter.*/

    Examples: 
      | target | option |
      | SW MGR | -m     |
      | SW 0x2 | -s 0x2 |
