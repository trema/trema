Feature: "dump_forward_entries" C API example command
  
  Switch Event forwarding configuration command (`dump_forward_entries`)
  is a command to dump event forwarding entries of 
  Switch Manager and Switch Daemons.
  
  The types of switch event which can be specified here are:
   vendor, packet_in, port_stat, and state_notify.
  
  This command can be used when: 
  * One needs to check which controller is handling certain event type.
  
  This command is also a simple usage example for event_forward_interface.h C API.
  The event_forward_interface.h API is used in topology daemon to 
  add itself to packet_in forwarding entry of all existing switch daemons and 
  switch manager to receive LLDP packets.
  By removing entry for 'topology' from some switches, it is possible to make 
  topology daemon to map only subset of all the switches managed by trema.

  Background: 
    Given I cd to "../../src/examples/switch_event_config/"
    Given I compile "dump_forward_entries.c" into "dump_forward_entries"

  Scenario: dump_forward_entries Usage
    When I run `trema run './dump_forward_entries -h'`
    Then the output should contain:
      """
      Dump OpenFlow Switch Manager/Daemon event forward entries.
       Switch Manager: dump_forward_entries -m -t EVENT_TYPE
       Switch Daemon : dump_forward_entries -s SWITCH_DPID -t EVENT_TYPE
      
       EVENT_TYPE:
        -t, --type={vendor,packet_in,port_status,state_notify} Specify event type.
      """

  Scenario Outline: Dump Switch Manager's event forward configuration for each type
    Given a file named "nw_dsl.conf" with:
      """
        vswitch { datapath_id 0x1 }
        vswitch { datapath_id 0x2 }
      """
    And I run `trema run ../repeater_hub/repeater-hub.rb -c nw_dsl.conf -d`
    And wait until "RepeaterHub" is up
    When I run `trema run './dump_forward_entries -m -t <type>'`
    Then the output should contain "Current service name list:"
    Then the output should contain "  RepeaterHub"

    Examples: 
      | type         |
      | vendor       |
      | packet_in    |
      | port_status  |
      | state_notify |

  Scenario Outline: Dump Switch Daemon's event forward configuration for each type on each switch
    Given a file named "nw_dsl.conf" with:
      """
        vswitch { datapath_id 0x1 }
        vswitch { datapath_id 0x2 }
      """
    And I run `trema run ../repeater_hub/repeater-hub.rb -c nw_dsl.conf -d`
    And wait until "RepeaterHub" is up
    When I run `trema run './dump_forward_entries -s <switch> -t <type>'`
    Then the output should contain "Current service name list:"
    Then the output should contain "  RepeaterHub"

    Examples: 
      | switch | type         |
      | 0x1    | vendor       |
      | 0x1    | packet_in    |
      | 0x1    | port_status  |
      | 0x1    | state_notify |
      | 0x2    | vendor       |
      | 0x2    | packet_in    |
      | 0x2    | port_status  |
      | 0x2    | state_notify |
