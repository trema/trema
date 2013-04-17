Feature: C function example command "dump_forward_entries"
  
  Switch Event forwarding configuration command (`dump_forward_entries`)
  is a command to dump event forwarding entries of 
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
    And I compile "dump_forward_entries.c" into "dump_forward_entries"

  Scenario: dump_forward_entries Usage
    When I successfully run `trema run './dump_forward_entries -h'`
    Then the output should contain:
      """
      Dump OpenFlow Switch Manager/Daemon event forward entries.
       Switch Manager: dump_forward_entries -m -t EVENT_TYPE
       Switch Daemon : dump_forward_entries -s SWITCH_DPID -t EVENT_TYPE
      
       EVENT_TYPE:
        -t, --type={vendor,packet_in,port_status,state_notify} Specify event type.
      """

  @slow_process
  Scenario Outline: Dump Switch Manager's event forwarding entries for each event type
    Given I successfully run `trema run ../repeater_hub/repeater-hub.rb -c network.conf -d`
    And wait until "RepeaterHub" is up
    When I successfully run `trema run './dump_forward_entries -m -t <type>'`
    Then the output should contain "Current service name list:"
    And the output should contain "  RepeaterHub"

    Examples: 
      | type         |
      | vendor       |
      | packet_in    |
      | port_status  |
      | state_notify |

  @slow_process
  Scenario Outline: Dump Switch Daemon's event forwarding entries for each event type on each switch
    Given I successfully run `trema run ../repeater_hub/repeater-hub.rb -c network.conf -d`
    And wait until "RepeaterHub" is up
    When I successfully run `trema run './dump_forward_entries -s <switch> -t <type>'`
    Then the output should contain "Current service name list:"
    And the output should contain "  RepeaterHub"

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
