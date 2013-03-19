Feature: Ruby methods for setting switch event forwarding entry.
  
  There are two Ruby methods provided for setting switch event forwarding entry.
  
  * set_forward_entries_to_switch
  * set_forward_entries_to_switch_manager
  
  These methods can be used by including the Trema::SwitchEvent module
  in user controller code. 
  
  ** set_forward_entries_to_switch dpid, event_type, trema_names **
  
  This method will set the forwarding entries of the switch specified by `dpid`. 
  It will replace the switch's 
  event forwarding entry list of the specified `event_type` 
  to Array of trema-names specified by `trema_names`. 
    
  
  ** set_forward_entries_to_switch_manager event_type, trema_names **
  
  This method will replace the switch manager's 
  event forwarding entry list of the specified `event_type`
  to Array of trema-names specified by `trema_names`. 
  
  ----
  All the above methods take a result handler as Ruby block, but 
  they can be omitted if checking is not necessary.

  Scenario Outline: set_forward_entries_to_switch dpid, event_type, trema_names
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      """
    And a file named "SetEntriesToSwitchDaemonTest.rb" with:
      """
      class SetEntriesToSwitchDaemonTest < Controller
        include SwitchEvent
      
        def switch_ready datapath_id
          set_forward_entries_to_switch datapath_id, <event_type>, ["SetEntriesToSwitchDaemonTest","Another"] do | success, services |
            raise "Failed to set forwarding entry to switch" if not success
            info "Successfully set a forwarding entries of <event_type> to switch #{datapath_id.to_hex} : #{services.inspect}"
          end
        end
      end
      """
    When I successfully run `trema run ./SetEntriesToSwitchDaemonTest.rb -c nw_dsl.conf -d`
    And wait until "SetEntriesToSwitchDaemonTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/SetEntriesToSwitchDaemonTest.log" should contain:
      """
      Successfully set a forwarding entries of <event_type> to switch 0x1 : [<switch_event_list>]
      """

    Examples: 
      | event_type    | switch_event_list                         |
      | :vendor       | "SetEntriesToSwitchDaemonTest", "Another" |
      | :packet_in    | "SetEntriesToSwitchDaemonTest", "Another" |
      | :port_status  | "SetEntriesToSwitchDaemonTest", "Another" |
      | :state_notify | "SetEntriesToSwitchDaemonTest", "Another" |

  Scenario Outline: set_forward_entries_to_switch_manager event_type, trema_names
    Given a file named "SetEntriesToSwitchManagerTest.rb" with:
      """
      class SetEntriesToSwitchManagerTest < Controller
        include SwitchEvent
      
        oneshot_timer_event :test_start, 0
        def test_start
          set_forward_entries_to_switch_manager <event_type>, ["SetEntriesToSwitchManagerTest","Another"] do | success, services |
            raise "Failed to set forwarding entry to switch manager" if not success
            info "Successfully set a forwarding entries of <event_type> to switch manager : #{services.inspect}"
          end
        end
      end
      """
    When I successfully run `trema run ./SetEntriesToSwitchManagerTest.rb -d`
    And wait until "SetEntriesToSwitchManagerTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/SetEntriesToSwitchManagerTest.log" should contain:
      """
      Successfully set a forwarding entries of <event_type> to switch manager : [<switch_manager_event_list>]
      """

    Examples: 
      | event_type    | switch_manager_event_list                  |
      | :vendor       | "SetEntriesToSwitchManagerTest", "Another" |
      | :packet_in    | "SetEntriesToSwitchManagerTest", "Another" |
      | :port_status  | "SetEntriesToSwitchManagerTest", "Another" |
      | :state_notify | "SetEntriesToSwitchManagerTest", "Another" |
