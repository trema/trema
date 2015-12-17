Welcome to Trema
================

[![Build Status](http://img.shields.io/travis/trema/trema/develop.svg?style=flat)][travis]
[![Code Climate](http://img.shields.io/codeclimate/github/trema/trema.svg?style=flat)][codeclimate]
[![Coverage Status](http://img.shields.io/codeclimate/coverage/github/trema/trema.svg?style=flat)][codeclimate]
[![Dependency Status](http://img.shields.io/gemnasium/trema/trema.svg?style=flat)][gemnasium]

Trema is an OpenFlow controller programming framework that provides
everything needed to create OpenFlow controllers in Ruby. It provides
a high-level OpenFlow library and also a network emulator that can
create OpenFlow-based networks for testing on your PC. This
self-contained environment helps streamlines the entire process of
development and testing.

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


Documentation
-------------

See https://relishapp.com/trema/trema/docs for links to documentation for all APIs.


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
* [trema/simple_router](https://github.com/trema/simple_router)
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
