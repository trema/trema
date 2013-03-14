Feature: Ruby APIs for adding switch event forwarding entry.
  
  There are 3 Ruby APIs provided for adding switch event forwarding entry:
  
  ** API to add an entry to all switches and switch manager **
  
      add_forward_entry_to_all_switches type, trema_name
  
  This API will add `trema_name` to all existing switches and switch manager's 
  event forwarding entry list for the specified switch event `type`.  
  
  ** API to add an entry to specified switch **
  
      add_forward_entry_to_switch dpid, type, trema_name
  
  This API will configure against the switch specified by `dpid`. 
  It will add `trema_name` to the switch's 
  event forwarding entry list for the specified switch event `type`.  
  
  ** API to add an entry to switch manager **
  
      add_forward_entry_to_switch_manager type, trema_name
  
  This API will add `trema_name` to the switch manager's 
  event forwarding entry list for the specified switch event `type`.  
  
  ----
  All the above APIs take result handler as Ruby block, but 
  they can be omitted if result is not necessary.  
  
  Please see README.md for general notes on switch event forwarding APIs.

  Scenario Outline: Add a switch event forwarding entry to all switches and switch manager for each event type
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      """
    And a file named "AddAllTest.rb" with:
      """
      class AddAllTest < Controller
        include SwitchEvent
      
        def switch_ready datapath_id
          oneshot_timer_event :test_start, 0
        end
      
        def test_start
          
          add_forward_entry_to_all_switches <event_type>, "added_entry" do | success |
            info "<event_type> result:#{success}"
            dump_forward_entries_from_switch 0x1, <event_type> do | success, services |
                info "<event_type> 0x1 result:#{success} services:#{services.inspect}"
            end
            dump_forward_entries_from_switch_manager <event_type> do | success, services |
                info "<event_type> manager result:#{success} services:#{services.inspect}"
            end
          end
        end
      end
      """
    When I run `trema run ./AddAllTest.rb -c nw_dsl.conf -d`
    And wait until "AddAllTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/AddAllTest.log" should contain:
      """
      <event_type> result:true
      """
    Then the file "../../tmp/log/AddAllTest.log" should contain:
      """
      <event_type> manager result:true services:[<sw_manager_event_list>]
      """
    Then the file "../../tmp/log/AddAllTest.log" should contain:
      """
      <event_type> 0x1 result:true services:[<sw_event_list>]
      """

    Examples: 
      | event_type    | sw_manager_event_list       | sw_event_list                                 |
      | :vendor       | "added_entry", "AddAllTest" | "added_entry", "AddAllTest"                   |
      | :packet_in    | "added_entry", "AddAllTest" | "added_entry", "AddAllTest"                   |
      | :port_status  | "added_entry", "AddAllTest" | "added_entry", "AddAllTest"                   |
      | :state_notify | "added_entry", "AddAllTest" | "added_entry", "AddAllTest", "switch_manager" |

  Scenario Outline: Add a switch event forwarding entry to specified switch for each event type
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      """
    And a file named "AddSwitchTest.rb" with:
      """
      class AddSwitchTest < Controller
        include SwitchEvent
      
        def switch_ready datapath_id
          oneshot_timer_event :test_start, 0
        end
      
        def test_start
          
          add_forward_entry_to_switch 0x1, <event_type>, "added_entry" do | success, services |
            info "<event_type> result:#{success} services:#{services.inspect}"
          end
        end
      end
      """
    When I run `trema run ./AddSwitchTest.rb -c nw_dsl.conf -d`
    And wait until "AddSwitchTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/AddSwitchTest.log" should contain:
      """
      <event_type> result:true services:[<sw_event_list>]
      """

    Examples: 
      | event_type    | sw_event_list                                    |
      | :vendor       | "added_entry", "AddSwitchTest"                   |
      | :packet_in    | "added_entry", "AddSwitchTest"                   |
      | :port_status  | "added_entry", "AddSwitchTest"                   |
      | :state_notify | "added_entry", "AddSwitchTest", "switch_manager" |

  Scenario Outline: Add a switch event forwarding entry to switch manager for each event type
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      """
    And a file named "AddSwitchManagerTest.rb" with:
      """
      class AddSwitchManagerTest < Controller
        include SwitchEvent
      
        def switch_ready datapath_id
          oneshot_timer_event :test_start, 0
        end
      
        def test_start
          
          add_forward_entry_to_switch_manager <event_type>, "added_entry" do | success, services |
            info "<event_type> result:#{success} services:#{services.inspect}"
          end
        end
      end
      """
    When I run `trema run ./AddSwitchManagerTest.rb -c nw_dsl.conf -d`
    And wait until "AddSwitchManagerTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/AddSwitchManagerTest.log" should contain:
      """
      <event_type> result:true services:[<sw_manager_event_list>]
      """

    Examples: 
      | event_type    | sw_manager_event_list                 |
      | :vendor       | "added_entry", "AddSwitchManagerTest" |
      | :packet_in    | "added_entry", "AddSwitchManagerTest" |
      | :port_status  | "added_entry", "AddSwitchManagerTest" |
      | :state_notify | "added_entry", "AddSwitchManagerTest" |
