Feature: Ruby methods for adding switch event forwarding entry
  
  There are three Ruby methods provided for adding switch event forwarding entries:
  
  * add_forward_entry_to_all_switches
  * add_forward_entry_to_switch
  * add_forward_entry_to_switch_manager
  
  These methods can be used by including the Trema::SwitchEvent module
  in user controller code. 
  
  * **add_forward_entry_to_all_switches event_type, trema_name**
  
  This method will add `trema_name` to all existing switches and switch manager's 
  event forwarding entry list of the specified `event_type`.  
  
  * **add_forward_entry_to_switch dpid, event_type, trema_name**
  
  This method will add an entry to a switch specified by `dpid`. 
  It will add `trema_name` to the switch's 
  event forwarding entry list of the specified `event_type`.  
  
  * **add_forward_entry_to_switch_manager event_type, trema_name**
  
  This method will add `trema_name` to the switch manager's 
  event forwarding entry list of the specified `event_type`.  
  
  ----
  All the above methods take a result handler as Ruby block, but 
  can be omitted if checking is not necessary.

  @slow_process
  Scenario Outline: add_forward_entry_to_all_switches event_type, trema_name
    Given a file named "network.conf" with:
      """
      vswitch { datapath_id 0x1 }
      """
    And a file named "AddEntryToAllTest.rb" with:
      """ruby
      class AddEntryToAllTest < Controller
        include SwitchEvent
        
        def start
          @event_type = ARGV[0].delete(":").to_sym
          @event_type_string = ":#{@event_type.to_s}"
        end
      
        def switch_ready datapath_id
          add_forward_entry_to_all_switches @event_type, "new_controller" do | success |
            raise "Failed to add forwarding entry to all switches" if not success
            info "Successfully added a forwarding entry of #{@event_type_string}."
            dump_forward_entries_from_switch datapath_id, @event_type do | success, services |
              raise "Failed to dump forwarding entry from a switch" if not success
              info "Dumping switch #{datapath_id.to_hex}'s forwarding entries of #{@event_type_string} : #{services.inspect}"
            end
            dump_forward_entries_from_switch_manager @event_type do | success, services |
              raise "Failed to dump forwarding entry from the switch manager" if not success
              info "Dumping switch manager's forwarding entries of #{@event_type_string} : #{services.inspect}"
            end
          end
        end
      end
      """
    When I successfully run `trema run "./AddEntryToAllTest.rb <event_type>" -c network.conf -d`
    And wait until "AddEntryToAllTest" is up
    And *** sleep 2 ***
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
      | event_type    | switch_manager_event_list             | switch_event_list                     |
      | :vendor       | "AddEntryToAllTest", "new_controller" | "AddEntryToAllTest", "new_controller" |
      | :packet_in    | "AddEntryToAllTest", "new_controller" | "AddEntryToAllTest", "new_controller" |
      | :port_status  | "AddEntryToAllTest", "new_controller" | "AddEntryToAllTest", "new_controller" |
      | :state_notify | "AddEntryToAllTest", "new_controller" | "AddEntryToAllTest", "new_controller" |

  @slow_process
  Scenario Outline: add_forward_entry_to_switch dpid, event_type, trema_name
    Given a file named "network.conf" with:
      """
      vswitch { datapath_id 0x1 }
      """
    And a file named "AddEntryToSwitchDaemonTest.rb" with:
      """ruby
      class AddEntryToSwitchDaemonTest < Controller
        include SwitchEvent
      
        def start
          @event_type = ARGV[0].delete(":").to_sym
          @event_type_string = ":#{@event_type.to_s}"
        end
      
        def switch_ready datapath_id
          add_forward_entry_to_switch datapath_id, @event_type, "new_controller" do | success, services |
            raise "Failed to add forwarding entry to switch" if not success
            info "Successfully added a forwarding entry of #{@event_type_string} to switch #{datapath_id.to_hex} : #{services.inspect}"
          end
        end
      end
      """
    When I successfully run `trema run "./AddEntryToSwitchDaemonTest.rb <event_type>" -c network.conf -d`
    And wait until "AddEntryToSwitchDaemonTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/AddEntryToSwitchDaemonTest.log" should contain:
      """
      Successfully added a forwarding entry of <event_type> to switch 0x1 : [<switch_event_list>]
      """

    Examples: 
      | event_type    | switch_event_list                              |
      | :vendor       | "AddEntryToSwitchDaemonTest", "new_controller" |
      | :packet_in    | "AddEntryToSwitchDaemonTest", "new_controller" |
      | :port_status  | "AddEntryToSwitchDaemonTest", "new_controller" |
      | :state_notify | "AddEntryToSwitchDaemonTest", "new_controller" |

  @slow_process
  Scenario Outline: add_forward_entry_to_switch_manager event_type, trema_name
    Given a file named "AddEntryToSwitchManagerTest.rb" with:
      """ruby
      class AddEntryToSwitchManagerTest < Controller
        include SwitchEvent
        
        def start
          @event_type = ARGV[0].delete(":").to_sym
          @event_type_string = ":#{@event_type.to_s}"
        end
      
        oneshot_timer_event :test_start, 0
        def test_start
          add_forward_entry_to_switch_manager @event_type, "new_controller" do | success, services |
            raise "Failed to add forwarding entry to switch manager" if not success
            info "Successfully added a forwarding entry of #{@event_type_string} to switch manager : #{services.inspect}"
          end
        end
      end
      """
    When I successfully run `trema run "./AddEntryToSwitchManagerTest.rb <event_type>" -d`
    And wait until "AddEntryToSwitchManagerTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/AddEntryToSwitchManagerTest.log" should contain:
      """
      Successfully added a forwarding entry of <event_type> to switch manager : [<switch_manager_event_list>]
      """

    Examples: 
      | event_type    | switch_manager_event_list                       |
      | :vendor       | "AddEntryToSwitchManagerTest", "new_controller" |
      | :packet_in    | "AddEntryToSwitchManagerTest", "new_controller" |
      | :port_status  | "AddEntryToSwitchManagerTest", "new_controller" |
      | :state_notify | "AddEntryToSwitchManagerTest", "new_controller" |
