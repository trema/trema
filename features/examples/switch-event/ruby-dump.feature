Feature: Ruby APIs for dumping switch event forwarding entry.
  
  There are 2 Ruby APIs provided for dumping switch event forwarding entry.
  
  ** API to dump the forwarding entries of the specified switch **
  
      dump_forward_entries_from_switch dpid, type
  
  This API will configure against the switch specified by `dpid`. 
  It will dump the content of the the switch's 
  event forwarding entry list for the specified switch event `type`.  
  
  ** API to dump the forwarding entries of the switch manager **
  
      dump_forward_entries_from_switch_manager type
  
  This API will dump the content of the the switch manager's 
  event forwarding entry list for the specified switch event `type`.  
  
  ----
  All the above APIs take result handler as Ruby block.  
  
  Please see README.md for general notes on switch event forwarding Ruby APIs.

  Scenario Outline: Dump the specified switch's event forwarding entry for each event type
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      event :vendor => "vendor", :packet_in => "packet_in", :port_status => "port_status", :state_notify => "state_notify"
      """
    And a file named "DumpSwitchTest.rb" with:
      """
      class DumpSwitchTest < Controller
        include SwitchEvent
      
        oneshot_timer_event :test_start, 0
      
        def test_start
          dump_forward_entries_from_switch 0x1, <event_type> do | success, services |
            info "<event_type> result:#{success} services:#{services.inspect}"
          end
        end
      end
      """
    When I run `trema run ./DumpSwitchTest.rb -c nw_dsl.conf -d`
    And wait until "DumpSwitchTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/DumpSwitchTest.log" should contain:
      """
      <event_type> result:true services:[<sw_event_list>]
      """

    Examples: 
      | event_type    | sw_event_list                    |
      | :vendor       | "vendor"                         |
      | :packet_in    | "packet_in"                      |
      | :port_status  | "port_status"                    |
      | :state_notify | "state_notify", "switch_manager" |

  Scenario Outline: Dump the switch manager's event forwarding entry for each event type
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      event :vendor => "vendor", :packet_in => "packet_in", :port_status => "port_status", :state_notify => "state_notify"
      """
    And a file named "DumpSwitchManagerTest.rb" with:
      """
      class DumpSwitchManagerTest < Controller
        include SwitchEvent
      
        oneshot_timer_event :test_start, 0
      
        def test_start
          dump_forward_entries_from_switch_manager <event_type> do | success, services |
            info "<event_type> result:#{success} services:#{services.inspect}"
          end
        end
      end
      """
    When I run `trema run ./DumpSwitchManagerTest.rb -c nw_dsl.conf -d`
    And wait until "DumpSwitchManagerTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/DumpSwitchManagerTest.log" should contain:
      """
      <event_type> result:true services:[<sw_manager_event_list>]
      """

    Examples: 
      | event_type    | sw_manager_event_list |
      | :vendor       | "vendor"              |
      | :packet_in    | "packet_in"           |
      | :port_status  | "port_status"         |
      | :state_notify | "state_notify"        |
