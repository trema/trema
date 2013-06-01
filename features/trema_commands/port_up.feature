Feature: port_up command

  Use this command to ensure that your controller detects network
  topology changes i.e., failures in switches.

  Trema emulated network offers a set of commands that can change the
  status of any arbitrary switch port. The `trema port_up` is one such
  command and its command syntax is as follows:

      $ trema port_up --switch DATAPATH_ID --port PORT_NUMBER

  The above command brings the switch's specified port up. By using
  this command one can easily test the :port_status handler defined in
  controllers.

  Background:
    Given a file named "sample.conf" with:
    """
    vswitch { datapath_id 0x1 }
    vswitch { datapath_id 0x2 }

    link "0x1", "0x2"
    """
    And a file named "port-observer.rb" with:
    """
    class PortObserver < Controller
      def switch_ready dpid
        info "Switch %#x is UP", dpid
      end


      def port_status dpid, message
        if message.phy_port.up?
          info "Port #{ message.phy_port.number } (Switch %#x) is UP", dpid
        elsif message.phy_port.down?
          info "Port #{ message.phy_port.number } (Switch %#x) is DOWN", dpid
        end
      end
    end
    """

  @slow_process
  Scenario: trema port_up --switch 0x1 --port 1
    Given I run `trema run ./port-observer.rb -c sample.conf` interactively
    And I wait for output to contain "Switch 0x1 is UP"
    And I run `trema port_down --switch 0x1 --port 1`
    And I wait for output to contain "Port 1 (Switch 0x1) is DOWN"
    When I run `trema port_up --switch 0x1 --port 1`
    Then the output should contain "Port 1 (Switch 0x1) is UP" within the timeout period

  @slow_process
  Scenario: trema port_up --switch UNKNOWN --port 1 (unknown switch error)
    Given I run `trema run ./port-observer.rb -c sample.conf -d`
    When I run `trema port_up --switch UNKNOWN --port 1`
    Then the output should contain "error: unknown switch: UNKNOWN"

  @slow_process
  Scenario: trema port_up --switch 0x1 --port 100 (unknown port error)
    Given I run `trema run ./port-observer.rb -c sample.conf -d`
    When I run `trema port_up --switch 0x1 --port 100`
    Then the output should contain "error: ovs-ofctl: vsw_0x1: couldn't find port `100'"

  @slow_process
  Scenario: trema port_up --switch 0x1 (no --port option error)
    Given I run `trema run ./port-observer.rb -c sample.conf -d`
    When I run `trema port_up --switch 0x1`
    Then the output should contain "error: --port option is mandatory"

  @slow_process
  Scenario: trema port_up --port 1 (no --switch option error)
    Given I run `trema run ./port-observer.rb -c sample.conf -d`
    When I run `trema port_up --port 1`
    Then the output should contain "error: --switch option is mandatory"

