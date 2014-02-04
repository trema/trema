Transparent firewall applications for Trema
-------------------------------------------

This example implements a transparent firewall, that is, a 2-port L2 switch that
passes or blocks Ethernet packets based on L3 (IPv4) criteria. The firewall is
intended for the following topology:

```
                        +-----+
                        |     |
                        | OFC |
                        |     |
                        +-----+
                           ^
                           |
                           v
                        +-----+
                        |     |
(Internet) ~---<-------># OFS #<------->---~ (LAN)
              "outside" |     | "inside"
                  port  +--#--+  port
                           |
                 "inspect" |
                     port  |
                           |
                           +--->---~ (/dev/zero)
```
In the current implementation the port numbers are hardcoded as follows:
* outside: port_no 1
* inside: port_no 2
* inspect: port_no 3

The "inspect" port serves as a kludge for the (existing) OFS implementations
that are unable to drop "any" IPv4 packets.

The files in this directory are:

* `block-rfc1918.rb` -- a Trema controller application that blocks in either
direction IPv4 packets that have source or destination address within the
[RFC1918](http://tools.ietf.org/html/rfc1918) private address space. All other
packets are permitted.
* `pass-delegated.rb` -- a Trema controller application that blocks from outside
to inside IPv4 packets that have source address outside of the globally
delegated IPv4 address space. All other packets are permitted. The application
runs as follows:

```
$ ./trema run src/examples/transparent_firewall/pass-delegated.rb
aggregated-delegated-afrinic.txt: 713 prefix(es)
aggregated-delegated-apnic.txt: 3440 prefix(es)
aggregated-delegated-arin.txt: 11342 prefix(es)
aggregated-delegated-lacnic.txt: 1937 prefix(es)
aggregated-delegated-ripencc.txt: 7329 prefix(es)
0x1000c425ec2dc: connected
0x1000c425ec2dc: bypass ON, loading started
0x1000c425ec2dc: bypass OFF, loading finished in 8.04 second(s)
```
* `regen_aggregated.sh` -- a helper shell script to regenerate prefix lists for
the above application (not required to run).
* `stats-to-cidrs.rb` -- a helper Ruby script for the above script.
* `aggregated-delegated-*.txt` -- files with IPv4 prefixes
