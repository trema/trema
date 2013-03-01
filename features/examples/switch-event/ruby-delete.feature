Feature: Ruby APIs for deleting switch event forwarding entry.
  
  There are 3 Ruby APIs provided for deleting switch event forwarding entry.
  
  ** API to delete an entry from all switches and switch manager **
  
      delete_forward_entry_from_all_switches type, trema_name
  
  This API will delete `trema_name` from all existing switches and switch manager's 
  event forwarding entry list for the specified switch event `type`.  
  
  ** API to delete an entry from specified switch **
  
      delete_forward_entry_from_switch dpid, type, trema_name
  
  This API will configure against the switch specified by `dpid`. 
  It will delete `trema_name` from the switch's 
  event forwarding entry list for the specified switch event `type`.  
  
  ** API to delete an entry from switch manager **
  
      delete_forward_entry_from_switch_manager type, trema_name
  
  This API will delete `trema_name` from the switch manager's 
  event forwarding entry list for the specified switch event `type`.  
  
  ----
  All the above APIs take result handler as Ruby block, but 
  they can be omitted if result is not necessary.  
  
  Please see README.md for general notes on switch event forwarding APIs.

  Scenario Outline: Delete a switch event forwarding entry from all switces and switch manager for each event type
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      """
    And a file named "DeleteAllTest.rb" with:
      """
      class DeleteAllTest < Controller
        include SwitchEvent
      
        def switch_ready datapath_id
          oneshot_timer_event :test_start, 0 if datapath_id == 0x1
        end
      
        def test_start
          
          delete_forward_entry_from_all_switches <event_type>, "DeleteAllTest" do | success |
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
    When I run `trema run ./DeleteAllTest.rb -c nw_dsl.conf -d`
    And wait until "DeleteAllTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/DeleteAllTest.log" should contain:
      """
      <event_type> result:true
      """
    Then the file "../../tmp/log/DeleteAllTest.log" should contain:
      """
      <event_type> manager result:true services:[<sw_manager_event_list>]
      """
    Then the file "../../tmp/log/DeleteAllTest.log" should contain:
      """
      <event_type> 0x1 result:true services:[<sw_event_list>]
      """

    Examples: 
      | event_type    | sw_manager_event_list | sw_event_list    |
      | :vendor       |                       |                  |
      | :packet_in    |                       |                  |
      | :port_status  |                       |                  |
      | :state_notify |                       | "switch_manager" |

  Scenario Outline: Delete a switch event forwarding entry from specified switch for each event type
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      """
    And a file named "DeleteSwitchTest.rb" with:
      """
      class DeleteSwitchTest < Controller
        include SwitchEvent
      
        def switch_ready datapath_id
          oneshot_timer_event :test_start, 0
        end
      
        def test_start
          
          delete_forward_entry_from_switch 0x1, <event_type>, "DeleteSwitchTest" do | success, services |
            info "<event_type> result:#{success} services:#{services.inspect}"
          end
        end
      end
      """
    When I run `trema run ./DeleteSwitchTest.rb -c nw_dsl.conf -d`
    And wait until "DeleteSwitchTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/DeleteSwitchTest.log" should contain:
      """
      <event_type> result:true services:[<sw_event_list>]
      """

    Examples: 
      | event_type    | sw_event_list    |
      | :vendor       |                  |
      | :packet_in    |                  |
      | :port_status  |                  |
      | :state_notify | "switch_manager" |

  Scenario Outline: Delete a switch event forwarding entry from switch manager for each event type
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      """
    And a file named "DeleteSwitchManagerTest.rb" with:
      """
      class DeleteSwitchManagerTest < Controller
        include SwitchEvent
      
        def switch_ready datapath_id
          oneshot_timer_event :test_start, 0
        end
      
        def test_start
          
          delete_forward_entry_from_switch_manager <event_type>, "DeleteSwitchManagerTest" do | success, services |
            info "<event_type> result:#{success} services:#{services.inspect}"
          end
        end
      end
      """
    When I run `trema run ./DeleteSwitchManagerTest.rb -c nw_dsl.conf -d`
    And wait until "DeleteSwitchManagerTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/DeleteSwitchManagerTest.log" should contain:
      """
      <event_type> result:true services:[<sw_manager_event_list>]
      """

    Examples: 
      | event_type    | sw_manager_event_list |
      | :vendor       |                       |
      | :packet_in    |                       |
      | :port_status  |                       |
      | :state_notify |                       |
