Feature: Ruby methods for adding switch event forwarding entry.
  
  There are three Ruby methods provided for adding switch event forwarding entries:
  
  * add_forward_entry_to_all_switches
  * add_forward_entry_to_switch
  * add_forward_entry_to_switch_manager
  
  These methods can be used by including the Trema::SwitchEvent module
  in user controller code. 
  
  ** add_forward_entry_to_all_switches event_type, trema_name **
  
  This method will add `trema_name` to all existing switches and switch manager's 
  event forwarding entry list of the specified `event_type`.  
  
  ** add_forward_entry_to_switch dpid, event_type, trema_name **
  
  This method will add an entry to a switch specified by `dpid`. 
  It will add `trema_name` to the switch's 
  event forwarding entry list of the specified `event_type`.  
  
  ** add_forward_entry_to_switch_manager event_type, trema_name **
  
  This method will add `trema_name` to the switch manager's 
  event forwarding entry list of the specified `event_type`.  
  
  ----
  All the above methods take a result handler as Ruby block, but 
  can be omitted if checking is not necessary.

  Scenario Outline: add_forward_entry_to_all_switches event_type, trema_name
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      """
    And a file named "AddEntryToAllTest.rb" with:
      """
      class AddEntryToAllTest < Controller
        include SwitchEvent
      
        def switch_ready datapath_id
          add_forward_entry_to_all_switches <event_type>, "new_controller" do | success |
            raise "Failed to add forwarding entry to all switches" if not success
            info "Successfully added a forwarding entry of <event_type>."
            dump_forward_entries_from_switch datapath_id, <event_type> do | success, services |
              raise "Failed to dump forwarding entry from a switch" if not success
              info "Dumping switch #{datapath_id.to_hex}'s forwarding entries of <event_type> : #{services.inspect}"
            end
            dump_forward_entries_from_switch_manager <event_type> do | success, services |
              raise "Failed to dump forwarding entry from the switch manager" if not success
              info "Dumping switch manager's forwarding entries of <event_type> : #{services.inspect}"
            end
          end
        end
      end
      """
    When I successfully run `trema run ./AddEntryToAllTest.rb -c nw_dsl.conf -d`
    And wait until "AddEntryToAllTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/AddEntryToAllTest.log" should contain:
      """
      Successfully added a forwarding entry of <event_type>.
      """
    And the file "../../tmp/log/AddEntryToAllTest.log" should contain:
      """
      Dumping switch manager's forwarding entries of <event_type> : [<switch_manager_event_list>]
      """
    And the file "../../tmp/log/AddEntryToAllTest.log" should contain:
      """
      Dumping switch 0x1's forwarding entries of <event_type> : [<switch_event_list>]
      """

    Examples: 
      | event_type    | switch_manager_event_list             | switch_event_list                                       |
      | :vendor       | "new_controller", "AddEntryToAllTest" | "new_controller", "AddEntryToAllTest"                   |
      | :packet_in    | "new_controller", "AddEntryToAllTest" | "new_controller", "AddEntryToAllTest"                   |
      | :port_status  | "new_controller", "AddEntryToAllTest" | "new_controller", "AddEntryToAllTest"                   |
      | :state_notify | "new_controller", "AddEntryToAllTest" | "new_controller", "AddEntryToAllTest", "switch_manager" |

  Scenario Outline: add_forward_entry_to_switch dpid, event_type, trema_name
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      """
    And a file named "AddEntryToSwitchDaemonTest.rb" with:
      """
      class AddEntryToSwitchDaemonTest < Controller
        include SwitchEvent
      
        def switch_ready datapath_id
          add_forward_entry_to_switch datapath_id, <event_type>, "new_controller" do | success, services |
            raise "Failed to add forwarding entry to switch" if not success
            info "Successfully added a forwarding entry of <event_type> to switch #{datapath_id.to_hex} : #{services.inspect}"
          end
        end
      end
      """
    When I successfully run `trema run ./AddEntryToSwitchDaemonTest.rb -c nw_dsl.conf -d`
    And wait until "AddEntryToSwitchDaemonTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/AddEntryToSwitchDaemonTest.log" should contain:
      """
      Successfully added a forwarding entry of <event_type> to switch 0x1 : [<switch_event_list>]
      """

    Examples: 
      | event_type    | switch_event_list                                                |
      | :vendor       | "new_controller", "AddEntryToSwitchDaemonTest"                   |
      | :packet_in    | "new_controller", "AddEntryToSwitchDaemonTest"                   |
      | :port_status  | "new_controller", "AddEntryToSwitchDaemonTest"                   |
      | :state_notify | "new_controller", "AddEntryToSwitchDaemonTest", "switch_manager" |

  Scenario Outline: add_forward_entry_to_switch_manager event_type, trema_name
    Given a file named "AddEntryToSwitchManagerTest.rb" with:
      """
      class AddEntryToSwitchManagerTest < Controller
        include SwitchEvent
        
        oneshot_timer_event :test_start, 0
        def test_start
          add_forward_entry_to_switch_manager <event_type>, "new_controller" do | success, services |
            raise "Failed to add forwarding entry to switch manager" if not success
            info "Successfully added a forwarding entry of <event_type> to switch manager : #{services.inspect}"
          end
        end
      end
      """
    When I successfully run `trema run ./AddEntryToSwitchManagerTest.rb -d`
    And wait until "AddEntryToSwitchManagerTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/AddEntryToSwitchManagerTest.log" should contain:
      """
      Successfully added a forwarding entry of <event_type> to switch manager : [<switch_manager_event_list>]
      """

    Examples: 
      | event_type    | switch_manager_event_list                       |
      | :vendor       | "new_controller", "AddEntryToSwitchManagerTest" |
      | :packet_in    | "new_controller", "AddEntryToSwitchManagerTest" |
      | :port_status  | "new_controller", "AddEntryToSwitchManagerTest" |
      | :state_notify | "new_controller", "AddEntryToSwitchManagerTest" |
