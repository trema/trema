Feature: netns
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "simple_hub.rb" with:
      """ruby
      class SimpleHub < Trema::Controller
        def switch_ready(dpid)
          send_flow_mod_add(
            dpid,
            match: Match.new,
            actions: SendOutPort.new(:flood)
          )
        end
      end
      """
    And a file named "simple_hub.conf" with:
      """ruby
      vswitch('simple_hub') { dpid 0x1 }
      netns('host1') {
        ip '192.168.1.2'
        netmask '255.255.255.0'
        route net: '0.0.0.0', gateway: '192.168.1.1'
      }
      netns('host2') {
        ip '192.168.1.3'
        netmask '255.255.255.0'
        route net: '0.0.0.0', gateway: '192.168.1.1'
      }
      link 'simple_hub', 'host1'
      link 'simple_hub', 'host2'
      """
    And I trema run "simple_hub.rb" with the configuration "simple_hub.conf"

  @sudo
  Scenario: netns namespece
    When I run `trema netns host1` interactively
    And I type "ip addr"
    And I type "exit"
    Then the stdout should contain "192.168.1.2"

  @sudo
  Scenario Outline: netns namespece command
    When I run `<command>`
    Then the stdout should contain "<message>"
    Examples:
      | command                                   | message                            |
      | trema netns host1 ip add show host1       | 192.168.1.2                        |
      | trema netns host1 -- ping -c1 192.168.1.3 | 1 packets transmitted, 1 received, |
      | trema netns host1 "ping -c1 192.168.1.3"  | 1 packets transmitted, 1 received, |
      | trema netns host1 "ip addr show lo"       | 127.0.0.1                          |
      | trema netns host1 ls $PWD                 | simple_hub.conf                    |
