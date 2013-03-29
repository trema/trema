Feature: C function example command "delete_forward_entry"
  
  Switch Event forwarding configuration command (`delete_forward_entry`)
  is a command to delete an event forwarding entry from 
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
    And I compile "delete_forward_entry.c" into "delete_forward_entry"
    And I compile "dump_forward_entries.c" into "dump_forward_entries"

  Scenario: delete_forward_entry Usage
    When I successfully run `trema run './delete_forward_entry -h'`
    Then the output should contain:
      """
      Delete OpenFlow Switch Manager/Daemon event forward entry.
       Both Switch Mgr/Daemon: delete_forward_entry -t EVENT_TYPE service_name
       Only Switch Manager   : delete_forward_entry -m -t EVENT_TYPE service_name
       Only Switch Daemon    : delete_forward_entry -s SWITCH_DPID -t EVENT_TYPE service_name
      
       EVENT_TYPE:
        -t, --type={vendor,packet_in,port_status,state_notify} Specify event type.
      """

  @slow_process
  Scenario Outline: Delete 'RepeaterHub' from All Switch Manager/Daemon's event forwarding entries of packet_in
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      vswitch { datapath_id 0x2 }
      """
    And I successfully run `trema run ../repeater_hub/repeater-hub.rb -c nw_dsl.conf -d`
    And wait until "RepeaterHub" is up
    When I successfully run `trema run './delete_forward_entry -t packet_in RepeaterHub '`
    Then the output should contain "Operation Succeeded."
    And I successfully run `trema run './dump_forward_entries <option> -t packet_in'`
    And the output should not match /Current service name list:.*  RepeaterHub/

    Examples: 
      | target | option |
      | SW MGR | -m     |
      | SW 0x1 | -s 0x1 |
      | SW 0x2 | -s 0x2 |

  @slow_process
  Scenario Outline: Delete 'RepeaterHub' only from Switch Manager's event forwarding entries of packet_in
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      vswitch { datapath_id 0x2 }
      """
    And I successfully run `trema run ../repeater_hub/repeater-hub.rb -c nw_dsl.conf -d`
    And wait until "RepeaterHub" is up
    When I successfully run `trema run './delete_forward_entry -m -t packet_in RepeaterHub'`
    Then the output should contain "Updated service name list is empty."
    And the output should not contain "  RepeaterHub"
    And I successfully run `trema run './dump_forward_entries -s <switch> -t packet_in'`
    And the output should match /Current service name list:.*  RepeaterHub/

    Examples: 
      | switch |
      | 0x1    |
      | 0x2    |

  @slow_process
  Scenario Outline: Delete 'RepeaterHub' only from Switch Daemon 0x1's event forwarding entries of packet_in
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      vswitch { datapath_id 0x2 }
      """
    And I successfully run `trema run ../repeater_hub/repeater-hub.rb -c nw_dsl.conf -d`
    And wait until "RepeaterHub" is up
    When I successfully run `trema run './delete_forward_entry -s 0x1 -t packet_in RepeaterHub'`
    Then the output should contain "Updated service name list is empty."
    And the output should not contain "  RepeaterHub"
    And I successfully run `trema run './dump_forward_entries <option> -t packet_in'`
    And the output should match /Current service name list:.*  RepeaterHub/

    Examples: 
      | target | option |
      | SW MGR | -m     |
      | SW 0x2 | -s 0x2 |
