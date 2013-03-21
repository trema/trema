@wip
Feature: Switch Port Specifier
  By adding a postfix (`:port#`) to the switch's name found in the `link` directive you can specify the exact port number to be used when the link is created.

      vswitch( "switch" ) { dpid 0xabc }
      vhost "host1"
      vhost "host2"
      vhost "host3"
      link "host1", "switch:1"  # Connects host1 to switch-port #1
      link "host2", "switch:2"  # Connects host2 to switch-port #2
      link "host3", "switch:3"  # Connects host3 to switch-port #3

  The reason why this feature required is that if the port number is not explicitly specified, trema randomly determines this. Which might be sufficient for some simple test cases but it is inadequate for other complex test cases.

  Therefore, if you wish to test your controllers rigorously on a configured virtual network you might find the `switch_name:port_number` syntax useful. It is also useful to be able to send packets to destined hosts or switches via the specified ports by using `SendOutPort` actions.
