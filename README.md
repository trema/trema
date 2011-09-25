Trema ![Project status](http://stillmaintained.com/trema/trema.png)
=====
An Open Source modular framework for developing OpenFlow controllers in Ruby/C


What's Trema?
-------------

Welcome to the Trema OpenFlow controller platform. The scope of Trema
is to help researchers and developers to easily develop their own
OpenFlow controllers, and NOT aiming at providing a specific OpenFlow
controller implementation. Trema provides various basic libraries on
which you can build your own OpenFlow controller, as well as
integrated network emulator and a lot of sample controllers written in
C and Ruby.


What's here?
------------

This distribution includes all the source code of Trema you need to
develop your own OpenFlow controllers. The source tree includes basic
libraries and functional modules that work as an interface to OpenFlow
switches. Several sample applications developed on top of Trema are
also provided, so you can run them as a sample of OpenFlow
controllers. Additionally, a simple but powerful framework that
emulates an OpenFlow-based network and end-hosts is provided for
testing your own controllers. For debugging, a wireshark plug-in to
diagnosis internal communications, e.g. RPC, among functional modules
is provided. Please refer to the quick start guide
(https://github.com/trema/trema/wiki/Quick-start) for details.


Meta
----

* Ruby API documents: http://rubydoc.info/github/trema/trema/master/frames
* Bugs: https://github.com/trema/trema/issues
* Mailing List: https://groups.google.com/group/trema-dev
* Twitter: http://twitter.com/trema_news
* Web Page: http://trema.github.com/trema/


Authors
-------

Please keep the list sorted.

* Kazushi Sugyo
* Kazuya Suzuki
* Lei Sun
* Naoyoshi Tada
* Nick Karanatsios <nickkaranatsios@gmail.com>
* Shin'ya Zenke
* Shuji Ishii
* Toshio Koide
* Yasuhito Takamiya <yasuhito@gmail.com>
* Yasunobu Chiba
* Yasunori Nakazawa


### Contributors

Thanks to:

- Initial Helios development team for implementing prototype versions.

- HIDEyuki Shimonishi for providing much-needed coffee, snacks, and christmas cakes!

- Anonymous alpha/beta testers for finding potential issues.


License & Terms
---------------

Copyright (C) 2008-2011 NEC Corporation

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.


### Terms

Terms of Contributing to Trema program ("Program")

Please read the following terms before you submit to the Trema project
("Project") any original works of corrections, modifications,
additions, patches and so forth to the Program ("Contribution").  By
submitting the Contribution, you are agreeing to be bound by the
following terms.  If you do not or cannot agree to any of the terms,
please do not submit the Contribution:

1. You hereby grant to any person or entity receiving or distributing
   the Program through the Project a worldwide, perpetual,
   non-exclusive, royalty free license to use, reproduce, modify,
   prepare derivative works of, display, perform, sublicense, and
   distribute the Contribution and such derivative works.

2. You warrant that you have all rights necessary to submit the
   Contribution and, to the best of your knowledge, the Contribution
   does not infringe copyright, patent, trademark, trade secret, or
   other intellectual property rights of any third parties.

3. In the event that the Contribution is combined with third parties'
   programs, you notify to Project maintainers including NEC
   Corporation ("Maintainers") the author, the license condition, and
   the name of such third parties' programs.

4. In the event that the Maintainers incorporates the Contribution
   into the Program, your name will not be indicated in the copyright
   notice of the Program, but will be indicated in the contributor
   list on the Project website.
