Welcome to Trema
================

[![Gem Version](https://badge.fury.io/rb/trema.png)](http://badge.fury.io/rb/trema)
[![Build Status](https://secure.travis-ci.org/trema/trema.png?branch=develop)](http://travis-ci.org/trema/trema)
[![Code Climate](https://codeclimate.com/github/trema/trema.png)](https://codeclimate.com/github/trema/trema)
[![Dependency Status](https://gemnasium.com/trema/trema.png)](https://gemnasium.com/trema/trema)

Trema is a OpenFlow controller framework that includes everything
needed to create OpenFlow controllers in Ruby and C.

This distribution includes all the source code of Trema you need to
develop your own OpenFlow controllers. The source tree includes basic
libraries and functional modules that work as an interface to OpenFlow
switches.

Several sample applications developed on top of Trema are also
provided, so you can run them as a sample of OpenFlow
controllers. Additionally, a simple but powerful framework that
emulates an OpenFlow-based network and end-hosts is provided for
testing your own controllers. For debugging, a wireshark plug-in to
diagnose internal data-flows among functional modules is provided.


Supported Platforms
-------------------

Trema supports GNU/Linux only. And it has been tested on the following environments:

* Ruby 1.8.7 or higher
* RubyGems 1.3.6 or higher
* Ubuntu 13.04, 12.10, 12.04, 11.10, and 10.04 (i386/amd64, Desktop Edition)
* Debian GNU/Linux 7.0 and 6.0 (i386/amd64)
* Fedora 16-19 (i386/x86_64)

It may also run on other GNU/Linux distributions but is not tested and
NOT SUPPORTED at this moment.


Supported OpenFlow Protocol Versions
------------------------------------

Trema currently supports OpenFlow version 1.0 only.


Getting Started
---------------

1.Install the prerequisites at the command prompt:

    (In Ubuntu or Debian GNU/Linux)
    $ sudo apt-get install gcc make ruby rubygems ruby-dev libpcap-dev libsqlite3-dev libglib2.0-dev

    (In Ubuntu 10.04)
    $ sudo apt-get install gcc make ruby rubygems ruby-dev libopenssl-ruby libpcap-dev libsqlite3-dev libglib2.0-dev
    $ sudo gem install rubygems-update --version 2.0.8
    $ sudo /var/lib/gems/1.8/bin/update_rubygems

    (In Fedora 16-19)
    $ sudo yum install gcc make ruby rubygems ruby-devel libpcap-devel libsqlite3x-devel glib2-devel

2.Install Trema at the command prompt:

    $ sudo gem install trema

3.Optional: Install Wireshark to diagnose internal data-flows.

    $ sudo apt-get install wireshark

4.Follow the guidelines to start developing your OpenFlow controller. You may find the following resources handy:

* The [Getting Started with Trema](https://github.com/trema/trema/wiki/Quick-start).
* The [Trema in 10 Minutes Tutorial](http://trema-10min.heroku.com/).
* The [Trema Tutorial](http://trema-tutorial.heroku.com/).
* The [Programming Trema Article (in Japanese)](http://gihyo.jp/dev/serial/01/openflow_sd/0007).


Ruby API
--------

The following is an excerpt from the Trema Ruby API.
The full documents are found here http://rubydoc.info/github/trema/trema/master/frames

### Event and Message Handlers

Subclass
[Trema::Controller](http://rubydoc.info/github/trema/trema/master/Trema/Controller)
and override some of the following methods to implement your own
controller.

```ruby
class MyController < Controller
  # handle Packet-In messages here.
  def packet_in datapath_id, message
    # ...
  end
  
  # handle Flow-Removed messages here.
  def flow_removed datapath_id, message
    # ...
  end
  
  # ...
end
```

* [switch_ready(datapath_id)](http://rubydoc.info/github/trema/trema/master/Trema/Controller:switch_ready)
* [switch_disconnected(datapath_id)](http://rubydoc.info/github/trema/trema/master/Trema/Controller:switch_disconnected)
* [list_switches_reply(datapath_ids)](http://rubydoc.info/github/trema/trema/master/Trema/Controller:list_switches_reply)
* [packet_in(datapath_id, message)](http://rubydoc.info/github/trema/trema/master/Trema/Controller:packet_in)
* [flow_removed(datapath_id, message)](http://rubydoc.info/github/trema/trema/master/Trema/Controller:flow_removed)
* [port_status(datapath_id, message)](http://rubydoc.info/github/trema/trema/master/Trema/Controller:port_status)
* [openflow_error(datapath_id, message)](http://rubydoc.info/github/trema/trema/master/Trema/Controller:openflow_error)
* [features_reply(datapath_id, message)](http://rubydoc.info/github/trema/trema/master/Trema/Controller:features_reply)
* [stats_reply(datapath_id, message)](http://rubydoc.info/github/trema/trema/master/Trema/Controller:stats_reply)
* [barrier_reply(datapath_id, message)](http://rubydoc.info/github/trema/trema/master/Trema/Controller:barrier_reply)
* [get_config_reply(datapath_id, message)](http://rubydoc.info/github/trema/trema/master/Trema/Controller:get_config_reply)
* [queue_get_config_reply(datapath_id, message)](http://rubydoc.info/github/trema/trema/master/Trema/Controller:queue_get_config_reply)
* [vendor(datapath_id, message)](http://rubydoc.info/github/trema/trema/master/Trema/Controller:vendor)

### Flow-Mod and Packet-Out

For sending Flow-Mod and Packet-Out, there are some methods defined in
[Trema::Controller](http://rubydoc.info/github/trema/trema/master/Trema/Controller)
class.

```ruby
class MyController < Controller
  def packet_in datapath_id, message
    # ...
    send_flow_mod_add( datapath_id, ... )
    send_packet_out( datapath_id, ... )
  end
  
  # ...
end
```

* [send_flow_mod_add(datapath_id, options)](http://rubydoc.info/github/trema/trema/master/Trema/Controller:send_flow_mod_add)
* [send_flow_mod_delete(datapath_id, options)](http://rubydoc.info/github/trema/trema/master/Trema/Controller:send_flow_mod_delete)
* [send_flow_mod_modify(datapath_id, options)](http://rubydoc.info/github/trema/trema/master/Trema/Controller:send_flow_mod_modify)
* [send_packet_out(datapath_id, options)](http://rubydoc.info/github/trema/trema/master/Trema/Controller:send_packet_out)

### Other OpenFlow Messages

The following OpenFlow messages can be sent with
[Trema::Controller#send_message](http://rubydoc.info/github/trema/trema/master/Trema/Controller:send_message)

```ruby
class MyController < Controller
  def switch_ready datapath_id
    # send a FeaturesRequest message
    send_message datapath_id, FeaturesRequest.new
  end
  
  def features_reply datapath_id, message
    # ...
  end
  
  # ...
end
```

* [Trema::Hello](http://rubydoc.info/github/trema/trema/master/Trema/Hello)
* [Trema::EchoRequest](http://rubydoc.info/github/trema/trema/master/Trema/EchoRequest)
* [Trema::EchoReply](http://rubydoc.info/github/trema/trema/master/Trema/EchoReply)
* [Trema::FeaturesRequest](http://rubydoc.info/github/trema/trema/master/Trema/FeaturesRequest)
* [Trema::SetConfig](http://rubydoc.info/github/trema/trema/master/Trema/SetConfig)
* [Trema::GetConfigRequest](http://rubydoc.info/github/trema/trema/master/Trema/GetConfigRequest)
* [Trema::QueueGetConfigRequest](http://rubydoc.info/github/trema/trema/master/Trema/QueueGetConfigRequest)
* [Trema::DescStatsRequest](http://rubydoc.info/github/trema/trema/master/Trema/DescStatsRequest)
* [Trema::FlowStatsRequest](http://rubydoc.info/github/trema/trema/master/Trema/FlowStatsRequest)
* [Trema::AggregateStatsRequest](http://rubydoc.info/github/trema/trema/master/Trema/AggregateStatsRequest)
* [Trema::TableStatsRequest](http://rubydoc.info/github/trema/trema/master/Trema/TableStatsRequest)
* [Trema::PortStatsRequest](http://rubydoc.info/github/trema/trema/master/Trema/PortStatsRequest)
* [Trema::QueueStatsRequest](http://rubydoc.info/github/trema/trema/master/Trema/QueueStatsRequest)
* [Trema::VendorStatsRequest](http://rubydoc.info/github/trema/trema/master/Trema/VendorStatsRequest)
* [Trema::BarrierRequest](http://rubydoc.info/github/trema/trema/master/Trema/BarrierRequest)
* [Trema::PortMod](http://rubydoc.info/github/trema/trema/master/Trema/PortMod)
* [Trema::Vendor](http://rubydoc.info/github/trema/trema/master/Trema/Vendor)

### Matching Rules

The matching rule of each flow table entry can be created with
[Match.new(options)](http://rubydoc.info/github/trema/trema/master/Trema/Match)
and passed as ":match =>" option when sending Flow-Mod or Packet-Out.

```ruby
def packet_in datapath_id, message
  # ...

  send_flow_mod_add(
    datapath_id,
    :match => Match.new( :in_port => message.in_port, ...)
    # ...
  )
  
  # ...
end
```

Also there is a utility method called
[ExactMatch.from(packetin)](http://rubydoc.info/github/trema/trema/master/Trema/ExactMatch)
for getting an exact match corresponding to a packet.

```ruby
def packet_in datapath_id, message
  # ...

  send_flow_mod_add(
    datapath_id,
    :match => ExactMatch.from( message )
    # ...
  )
  
  # ...
end
```

### Actions

The actions list of each flow table entry can be set with ":actions
=>" when sending Flow-Mod or Packet-Out.

```ruby
# Strip the VLAN tag of a packet then send it out to switch port #1
send_flow_mod_add(
  datapath_id,
  # ...
  :actions => [ StripVlanHeader.new, SendOutPort.new( 1 ) ]
)
```

* [Trema::SendOutPort](http://rubydoc.info/github/trema/trema/master/Trema/SendOutPort)
* [Trema::SetEthSrcAddr](http://rubydoc.info/github/trema/trema/master/Trema/SetEthSrcAddr)
* [Trema::SetEthDstAddr](http://rubydoc.info/github/trema/trema/master/Trema/SetEthDstAddr)
* [Trema::SetIpSrcAddr](http://rubydoc.info/github/trema/trema/master/Trema/SetIpSrcAddr)
* [Trema::SetIpDstAddr](http://rubydoc.info/github/trema/trema/master/Trema/SetIpDstAddr)
* [Trema::SetIpTos](http://rubydoc.info/github/trema/trema/master/Trema/SetIpTos)
* [Trema::SetTransportSrcPort](http://rubydoc.info/github/trema/trema/master/Trema/SetTransportSrcPort)
* [Trema::SetTransportDstPort](http://rubydoc.info/github/trema/trema/master/Trema/SetTransportDstPort)
* [Trema::SetVlanVid](http://rubydoc.info/github/trema/trema/master/Trema/SetVlanVid)
* [Trema::SetVlanPriority](http://rubydoc.info/github/trema/trema/master/Trema/SetVlanPriority)
* [Trema::StripVlanHeader](http://rubydoc.info/github/trema/trema/master/Trema/StripVlanHeader)
* [Trema::VendorAction](http://rubydoc.info/github/trema/trema/master/Trema/VendorAction)


Meta
----

* Web Page: http://trema.github.com/trema/
* Bugs: https://github.com/trema/trema/issues
* Mailing List: https://groups.google.com/group/trema-dev
* Twitter: http://twitter.com/trema_news


Contributors
------------

Special thanks to all contributors for submitting patches. A full list
of contributors including their patches can be found at:

https://github.com/trema/trema/contributors


License
-------

Trema is released under the GNU General Public License version 2.0:

* http://www.gnu.org/licenses/gpl-2.0.html
