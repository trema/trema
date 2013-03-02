Feature: "add_forward_entry" C API example command
  
  Switch Event forwarding configuration command (`add_forward_entry`)
  is a command to add an event forwarding entry to 
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
    Given I compile "add_forward_entry.c" into "add_forward_entry"
    Given I compile "dump_forward_entries.c" into "dump_forward_entries"

  Scenario: add_forward_entry Usage
    When I run `trema run './add_forward_entry -h'`
    Then the output should contain:
      """
      Add OpenFlow Switch Manager/Daemon event forward entry.
       Both Switch Mgr/Daemon: add_forward_entry -t EVENT_TYPE service_name
       Only Switch Manager   : add_forward_entry -m -t EVENT_TYPE service_name
       Only Switch Daemon    : add_forward_entry -s SWITCH_DPID -t EVENT_TYPE service_name
      
       EVENT_TYPE:
        -t, --type={vendor,packet_in,port_status,state_notify} Specify event type.
      """

  Scenario Outline: Add 'mirror' to All Switch Manager/Daemon's event forward configuration for packet_in
    Given a file named "nw_dsl.conf" with:
      """
        vswitch { datapath_id 0x1 }
        vswitch { datapath_id 0x2 }
      """
    And I run `trema run ../repeater_hub/repeater-hub.rb -c nw_dsl.conf -d`
    And wait until "RepeaterHub" is up
    When I run `trema run './add_forward_entry -t packet_in mirror'`
    Then the output should contain "Operation Succeeded."
    Then I run `trema run './dump_forward_entries <option> -t packet_in'`
    Then the output should contain "Current service name list:"
    Then the output should match /.*  RepeaterHub.*/
    Then the output should match /.*  mirror.*/

    Examples: 
      | target | option |
      | SW MGR | -m     |
      | SW 0x1 | -s 0x1 |
      | SW 0x2 | -s 0x2 |

  Scenario Outline: Switch added after event forward configuration manipulation should also reflect new configuration.
    Given a file named "nw_dsl.conf" with:
      """
        vswitch { datapath_id 0x1 }
        vswitch { datapath_id 0x2 }
      """
    And I run `trema run ../repeater_hub/repeater-hub.rb -c nw_dsl.conf -d`
    And wait until "RepeaterHub" is up
    Given I run `trema run './add_forward_entry -t packet_in mirror'`
    Given the output should contain "Operation Succeeded."
    Given a file named "switch_event_config_add.conf" with:
      """
      vswitch { datapath_id 0x3 }
      """
    Given I run `trema run -c switch_event_config_add.conf -d`
    When I run `trema run './dump_forward_entries <option> -t packet_in'`
    Then the output should contain "Current service name list:"
    Then the output should match /.*  RepeaterHub.*/
    Then the output should match /.*  mirror.*/

    Examples: 
      | target | option |
      | SW MGR | -m     |
      | SW 0x1 | -s 0x1 |
      | SW 0x2 | -s 0x2 |
      | SW 0x3 | -s 0x3 |

  Scenario Outline: Add 'mirror' only to Switch Manager's event forward configuration for packet_in
    Given a file named "nw_dsl.conf" with:
      """
        vswitch { datapath_id 0x1 }
        vswitch { datapath_id 0x2 }
      """
    And I run `trema run ../repeater_hub/repeater-hub.rb -c nw_dsl.conf -d`
    And wait until "RepeaterHub" is up
    When I run `trema run './add_forward_entry -m -t packet_in mirror'`
    Then the output should contain "Updated service name list:"
    Then the output should contain "  RepeaterHub"
    Then the output should contain "  mirror"
    Then I run `trema run './dump_forward_entries -s <switch> -t packet_in'`
    Then the output should match /Current service name list:.*  RepeaterHub/
    Then the output should not match /Current service name list:.*  mirror/

    Examples: 
      | switch |
      | 0x1    |
      | 0x2    |

  Scenario Outline: Add 'mirror' only to Switch Daemon 0x1's event forward configuration for packet_in
    Given a file named "nw_dsl.conf" with:
      """
        vswitch { datapath_id 0x1 }
        vswitch { datapath_id 0x2 }
      """
    And I run `trema run ../repeater_hub/repeater-hub.rb -c nw_dsl.conf -d`
    And wait until "RepeaterHub" is up
    When I run `trema run './add_forward_entry -s 0x1 -t packet_in mirror'`
    Then the output should contain "Updated service name list:"
    Then the output should contain "  RepeaterHub"
    Then the output should contain "  mirror"
    Then I run `trema run './dump_forward_entries <option> -t packet_in'`
    Then the output should match /Current service name list:.*  RepeaterHub/
    Then the output should not match /Current service name list:.*  mirror/

    Examples: 
      | target | option |
      | SW MGR | -m     |
      | SW 0x2 | -s 0x2 |
