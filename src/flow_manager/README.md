Flow manager
============

**!!!! NOTICE !!!!**
**THIS APPLICATION IS STILL UNDER DEVELOPMENT AND MAY NOT WORK AS EXPECTED.**

What's this?
------------

Flow manage provides high-level APIs on the raw OpenFlow protocol layer 
for managing multiple flow entries as a single set. 
If you want to install flow entries across multiple switches, 
flow manager guarantees that all entries are properly installed into the switches.
 If any error occurs, all entries associated with the set are removed from the switches. 
If all entries are removed from the switches, a requester is notified from flow manager.

Software architecture/hierarchy is as follows:

          
      +-----+
      | App |
      +-----+
         | 
      +----------------------------------------+
      |     path(Flow Manager) API          |
      +----------------------------------------+
         |  Flow Manager Interface
         |  Messanger service
      +------------------------------+
      |     Flow Manager + Trema     |
      +------------------------------+


path which cooperates with flow manager allows you to set up/tear
down/look up a path which is a set of flow entries that share the same
packet match criteria easily.

How to use
----------

Please refer to examples as references for understanding
how to use the APIs.

How to build
------------

  Build Trema. This software is included into the Trema.

        $ ./build.rb

How to run examples
-------------------

    ruby example :
    $ ./trema run -c src/examples/flow_manager_example/flow_manager_ruby.conf src/examples/flow_manager_example/flow_manager.rb --flowmanager

    or

    c example:
    $ ./trema run -c src/examples/flow_manager_example/flow_manager_test.conf object/examples/flow_manager_example/flow_manager_test --flowmanager

All examples show how to setup or teardown paths.

License & Terms
---------------

Copyright (C) 2011 NEC Corporation

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.


## Terms

Terms of Contributing to Trema program ("Program")

Please read the following terms before you submit to the Trema project
("Project") any original works of corrections, modifications,
additions, patches and so forth to the Program ("Contribution"). By
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
