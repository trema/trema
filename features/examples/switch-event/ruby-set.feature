Feature: Ruby APIs for setting switch event forwarding entry.
  
  There are 2 Ruby APIs provided for setting switch event forwarding entry.
  
  ** API to set the forwarding entries of the specified switch **
  
      set_forward_entries_to_switch dpid, type, trema_names
  
  This API will configure against the switch specified by `dpid`. 
  It will replace the switch's 
  event forwarding entry list to the Array of trema-names specified by `trema_names` 
  for the specified switch event `type`.  
  
  ** API to set the forwarding entries of the switch manager **
  
      set_forward_entries_to_switch_manager type, trema_names
  
  This API will replace the switch manager's 
  event forwarding entry list to the Array of trema-names specified by `trema_names` 
  for the specified switch event `type`.  
  
  ----
  All the above APIs take result handler as Ruby block, but 
  they can be omitted if result is not necessary.  
  
  Please see README.md for general notes on switch event forwarding APIs.

  Scenario Outline: Set the switch event forwarding entries of the specified switch for each event type
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      """
    And a file named "SetSwitchTest.rb" with:
      """
      class SetSwitchTest < Controller
        include SwitchEvent
      
        def switch_ready datapath_id
          oneshot_timer_event :test_start, 0
        end
      
        def test_start
          
          set_forward_entries_to_switch 0x1, <event_type>, ["SetSwitchTest","Another"] do | success, services |
            info "<event_type> result:#{success} services:#{services.inspect}"
          end
        end
      end
      """
    When I run `trema run ./SetSwitchTest.rb -c nw_dsl.conf -d`
    And wait until "SetSwitchTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/SetSwitchTest.log" should contain:
      """
      <event_type> result:true services:[<sw_event_list>]
      """

    Examples: 
      | event_type    | sw_event_list              |
      | :vendor       | "SetSwitchTest", "Another" |
      | :packet_in    | "SetSwitchTest", "Another" |
      | :port_status  | "SetSwitchTest", "Another" |
      | :state_notify | "SetSwitchTest", "Another" |

  Scenario Outline: Set the switch event forwarding entries of the switch manager for each event type
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      """
    And a file named "SetSwitchManagerTest.rb" with:
      """
      class SetSwitchManagerTest < Controller
        include SwitchEvent
      
        def switch_ready datapath_id
          info "switch_ready %#x" % datapath_id
          oneshot_timer_event :test_start, 0 if datapath_id == 0x1
        end
      
        def test_start
          
          set_forward_entries_to_switch_manager <event_type>, ["SetSwitchManagerTest","Another"] do | success, services |
            info "<event_type> result:#{success} services:#{services.inspect}"
          end
        end
      end
      """
    When I run `trema run ./SetSwitchManagerTest.rb -c nw_dsl.conf -d`
    And wait until "SetSwitchManagerTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/SetSwitchManagerTest.log" should contain:
      """
      <event_type> result:true services:[<sw_manager_event_list>]
      """

    Examples: 
      | event_type    | sw_manager_event_list             |
      | :vendor       | "SetSwitchManagerTest", "Another" |
      | :packet_in    | "SetSwitchManagerTest", "Another" |
      | :port_status  | "SetSwitchManagerTest", "Another" |
      | :state_notify | "SetSwitchManagerTest", "Another" |
