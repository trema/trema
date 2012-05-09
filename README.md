Welcome to Trema
================

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


Getting Started
---------------

1.Install the prerequisites at the command prompt:

    $ sudo apt-get install gcc make ruby rubygems ruby-dev irb file libpcap-dev libsqlite3-dev

2.Install Trema at the command prompt:

    $ sudo gem install trema

3.Follow the guidelines to start developing your OpenFlow controller. You may find the following resources handy:

* The [Getting Started with Trema](https://github.com/trema/trema/wiki/Quick-start).
* The [Trema in 10 Minutes Tutorial](http://trema-10min.heroku.com/).
* The [Trema Tutorial](http://trema-tutorial.heroku.com/).
* The [Trema Ruby API documents](http://rubydoc.info/github/trema/trema/master/frames).
* The [Programming Trema Article (in Japanese)](http://gihyo.jp/dev/serial/01/openflow_sd/0007).


Meta
----

* Web Page: http://trema.github.com/trema/
* Bugs: https://github.com/trema/trema/issues
* Mailing List: https://groups.google.com/group/trema-dev
* Twitter: http://twitter.com/trema_news


Supported Platforms
-------------------

Trema has been tested ONLY on the following environments:

* Ubuntu 12.04, 11.10, 11.04, 10.10, and 10.04 (i386/amd64, Desktop Edition)
* Debian GNU/Linux 6.0 (i386/amd64)

It may also run on other GNU/Linux distributions but is not tested and
NOT SUPPORTED at this moment.


Contributors
------------

Special thanks to all contributors for submitting patches. A full list of contributors including their patches can be found at:

https://github.com/trema/trema/contributors


Project Status
--------------

* Build Status [![Build Status](https://secure.travis-ci.org/trema/trema.png?branch=develop)](http://travis-ci.org/trema/trema)
* Dependency Status [![Dependency Status](https://gemnasium.com/trema/trema.png)](https://gemnasium.com/trema/trema)


License
-------

Trema is released under the GNU General Public License version 2.0:

* http://www.gnu.org/licenses/gpl-2.0.html
