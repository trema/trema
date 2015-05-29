Welcome to Trema
================

[![Build Status](http://img.shields.io/travis/trema/trema/develop.svg?style=flat)][travis]
[![Code Climate](http://img.shields.io/codeclimate/github/trema/trema.svg?style=flat)][codeclimate]
[![Coverage Status](http://img.shields.io/codeclimate/coverage/github/trema/trema.svg?style=flat)][codeclimate]
[![Dependency Status](http://img.shields.io/gemnasium/trema/trema.svg?style=flat)][gemnasium]

Trema is a OpenFlow controller framework that includes everything
needed to create OpenFlow controllers in Ruby.

This distribution includes all the source code of Trema you need to
develop your own OpenFlow controllers. The source tree includes basic
libraries and functional modules that work as an interface to OpenFlow
switches.

Several sample applications developed on top of Trema are also
provided, so you can run them as a sample of OpenFlow
controllers. Additionally, a simple but powerful framework that
emulates an OpenFlow-based network and end-hosts is provided for
testing your own controllers.

[travis]: http://travis-ci.org/trema/trema
[codeclimate]: https://codeclimate.com/github/trema/trema
[gemnasium]: https://gemnasium.com/trema/trema
[gitter]: https://gitter.im/trema/trema


Prerequisites
-------------

* Ruby 2.0.0 or higher ([RVM][rvm]).
* [Open vSwitch][openvswitch] (`apt-get install openvswitch-switch`).

[rvm]: https://rvm.io/
[openvswitch]: https://openvswitch.org/


Sample Code
-----------

Study sample code for implementation examples of Trema features. Each
sample code project is executable source example of how to write a
OpenFlow controller using Trema Ruby API.

* [trema/hello_trema](https://github.com/trema/hello_trema)
* [trema/repeater_hub](https://github.com/trema/repeater_hub)
* [trema/patch_panel](https://github.com/trema/patch_panel)
* [trema/cbench](https://github.com/trema/cbench)
* [trema/learning_switch](https://github.com/trema/learning_switch)
* [trema/switch_monitor](https://github.com/trema/switch_monitor)
* [trema/topology](https://github.com/trema/topology)
* [trema/routing_switch](https://github.com/trema/routing_switch)


Contributors
------------

Special thanks to all contributors for submitting patches. A full list
of contributors including their patches can be found at:

https://github.com/trema/trema/contributors


License
-------

Trema is released under the GNU General Public License version 2.0:

* http://www.gnu.org/licenses/gpl-2.0.html
