Feature: Ruby methods for deleting switch event forwarding entry.
  
  There are three Ruby methods provided for deleting switch event forwarding entries.
  
  * delete_forward_entry_from_all_switches
  * delete_forward_entry_from_switch
  * delete_forward_entry_from_switch_manager
  
  These methods can be used by including the Trema::SwitchEvent module
  in user controller code. 
  
  ** delete_forward_entry_from_all_switches event_type, trema_name **
  
  This method will delete `trema_name` from all existing switches and switch manager's 
  event forwarding entry list of the specified `event_type`.  
  
  ** delete_forward_entry_from_switch dpid, event_type, trema_name **
  
  This method will delete an entry from switch specified by `dpid`. 
  It will delete `trema_name` from the switch's 
  event forwarding entry list of the specified `event_type`.  
  
  ** delete_forward_entry_from_switch_manager event_type, trema_name **
  
  This method will delete `trema_name` from the switch manager's 
  event forwarding entry list of the specified `event_type`.  
  
  ----
  All the above methods take a result handler as Ruby block, but 
  they can be omitted if checking is not necessary.

  Scenario Outline: delete_forward_entry_from_all_switches event_type, trema_name
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      """
    And a file named "DeleteEntryFromAllTest.rb" with:
      """
      class DeleteEntryFromAllTest < Controller
        include SwitchEvent
      
        def switch_ready datapath_id
          delete_forward_entry_from_all_switches <event_type>, "DeleteEntryFromAllTest" do | success |
            raise "Failed to delete forwarding entry from all switches" if not success
            info "Successfully deleted a forwarding entry of <event_type>."
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
    When I successfully run `trema run ./DeleteEntryFromAllTest.rb -c nw_dsl.conf -d`
    And wait until "DeleteEntryFromAllTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/DeleteEntryFromAllTest.log" should contain:
      """
      Successfully deleted a forwarding entry of <event_type>.
      """
    And the file "../../tmp/log/DeleteEntryFromAllTest.log" should contain:
      """
      Dumping switch 0x1's forwarding entries of <event_type> : [<switch_event_list>]
      """
    And the file "../../tmp/log/DeleteEntryFromAllTest.log" should contain:
      """
      Dumping switch manager's forwarding entries of <event_type> : [<switch_manager_event_list>]
      """

    Examples: 
      | event_type    | switch_manager_event_list | switch_event_list |
      | :vendor       |                           |                   |
      | :packet_in    |                           |                   |
      | :port_status  |                           |                   |
      | :state_notify |                           | "switch_manager"  |

  Scenario Outline: delete_forward_entry_from_switch dpid, event_type, trema_name
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      """
    And a file named "DeleteFromSwitchDaemonTest.rb" with:
      """
      class DeleteFromSwitchDaemonTest < Controller
        include SwitchEvent
      
        def switch_ready datapath_id
          delete_forward_entry_from_switch datapath_id, <event_type>, "DeleteFromSwitchDaemonTest" do | success, services |
            raise "Failed to delete forwarding entry from switch" if not success
            info "Successfully deleted a forwarding entry of <event_type> from switch #{datapath_id.to_hex} : #{services.inspect}"
          end
        end
      end
      """
    When I successfully run `trema run ./DeleteFromSwitchDaemonTest.rb -c nw_dsl.conf -d`
    And wait until "DeleteFromSwitchDaemonTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/DeleteFromSwitchDaemonTest.log" should contain:
      """
      Successfully deleted a forwarding entry of <event_type> from switch 0x1 : [<switch_event_list>]
      """

    Examples: 
      | event_type    | switch_event_list |
      | :vendor       |                   |
      | :packet_in    |                   |
      | :port_status  |                   |
      | :state_notify | "switch_manager"  |

  Scenario Outline: delete_forward_entry_from_switch_manager event_type, trema_name
    Given a file named "DeleteFromSwitchManagerTest.rb" with:
      """
      class DeleteFromSwitchManagerTest < Controller
        include SwitchEvent
      
        oneshot_timer_event :test_start, 0
        def test_start
          delete_forward_entry_from_switch_manager <event_type>, "DeleteFromSwitchManagerTest" do | success, services |
            raise "Failed to delete forwarding entry from switch manager" if not success
            info "Successfully deleted a forwarding entry of <event_type> from switch manager : #{services.inspect}"
          end
        end
      end
      """
    When I successfully run `trema run ./DeleteFromSwitchManagerTest.rb -d`
    And wait until "DeleteFromSwitchManagerTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/DeleteFromSwitchManagerTest.log" should contain:
      """
      Successfully deleted a forwarding entry of <event_type> from switch manager : [<switch_manager_event_list>]
      """

    Examples: 
      | event_type    | switch_manager_event_list |
      | :vendor       |                           |
      | :packet_in    |                           |
      | :port_status  |                           |
      | :state_notify |                           |
