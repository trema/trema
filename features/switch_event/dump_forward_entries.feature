Feature: Ruby methods for dumping switch event forwarding entry
  
  There are two Ruby methods provided for dumping switch event forwarding entry.
  
  * dump_forward_entries_from_switch
  * dump_forward_entries_from_switch_manager
  
  These methods can be used by including the Trema::SwitchEvent module
  in user controller code. 
  
  * **dump_forward_entries_from_switch dpid, event_type**
  
  This method will dump the forwarding entries of the switch specified by `dpid`. 
  It will dump the content of the the switch's 
  event forwarding entry list of the specified `event_type`.  
  
  * **dump_forward_entries_from_switch_manager event_type**
  
  This method will dump the content of the the switch manager's 
  event forwarding entry list of the specified `event_type`.  
  
  ----
  All the above methods take a result handler as Ruby block.

  Scenario Outline: dump_forward_entries_from_switch dpid, event_type
    Given a file named "nw_dsl.conf" with:
      """
      vswitch { datapath_id 0x1 }
      event :vendor => "vendor", :packet_in => "packet_in", :port_status => "port_status"
      """
    And a file named "DumpSwitchDaemonTest.rb" with:
      """ruby
      class DumpSwitchDaemonTest < Controller
        include SwitchEvent
      
        def switch_ready datapath_id
          dump_forward_entries_from_switch datapath_id, <event_type> do | success, services |
            raise "Failed to dump forwarding entry from a switch" if not success
            info "Dumping switch #{datapath_id.to_hex}'s forwarding entries of <event_type> : #{services.inspect}"
          end
        end
      end
      """
    When I successfully run `trema run ./DumpSwitchDaemonTest.rb -c nw_dsl.conf -d`
    And wait until "DumpSwitchDaemonTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/DumpSwitchDaemonTest.log" should contain:
      """
      Dumping switch 0x1's forwarding entries of <event_type> : [<switch_event_list>]
      """

    Examples: 
      | event_type    | switch_event_list                        |
      | :vendor       | "vendor"                                 |
      | :packet_in    | "packet_in"                              |
      | :port_status  | "port_status"                            |
      | :state_notify | "DumpSwitchDaemonTest", "switch_manager" |

  Scenario Outline: dump_forward_entries_from_switch_manager event_type
    Given a file named "nw_dsl.conf" with:
      """
      event :vendor => "vendor", :packet_in => "packet_in", :port_status => "port_status", :state_notify => "state_notify"
      """
    And a file named "DumpSwitchManagerTest.rb" with:
      """ruby
      class DumpSwitchManagerTest < Controller
        include SwitchEvent
      
        oneshot_timer_event :test_start, 0
        def test_start
          dump_forward_entries_from_switch_manager <event_type> do | success, services |
            raise "Failed to dump forwarding entry from the switch manager" if not success
            info "Dumping switch manager's forwarding entries of <event_type> : #{services.inspect}"
          end
        end
      end
      """
    When I successfully run `trema run ./DumpSwitchManagerTest.rb -c nw_dsl.conf -d`
    And wait until "DumpSwitchManagerTest" is up
    And *** sleep 1 ***
    Then the file "../../tmp/log/DumpSwitchManagerTest.log" should contain:
      """
      Dumping switch manager's forwarding entries of <event_type> : [<switch_manager_event_list>]
      """

    Examples: 
      | event_type    | switch_manager_event_list |
      | :vendor       | "vendor"                  |
      | :packet_in    | "packet_in"               |
      | :port_status  | "port_status"             |
      | :state_notify | "state_notify"            |
