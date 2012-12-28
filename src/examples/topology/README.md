Topology Examples
=================

This directory includes sample application using libtopology.

- `show_topology` is a command line application, which retrieves 
  link information from Topology daemon and print them in trema network DSL style.

- `show_switch_status` is a command line application, which retrieves 
  switch and port information from Topology daemon and print them to stdout.

- `enable_discovery` is a command line application, which enables
  link discovery feature using the topology API.


        +----------+           +-----------+           +------+    +------------------+
        |  switch  |  *    1   | packet in |  1    *   |dumper|    |show_topology/    |
        |  daemon  | --------> |  filter   | --------> |      |    |show_switch_status|
        +----------+ packet in +-----------+ packet in +------+    +------------------+
          ^ 1    ^                       |                               * ^ 
          |      |                       |                                 | 
          |      `-------.               |                    topology API |
          v 1            |               |                               1 v 
        +----------+     |               |   packet in(LLDP)        +-----------+
        | openflow |     |               `------------------------->| topology  |
        |  switch  |     `------------------------------------------|  daemon   |
        +----------+                         packet out(LLDP)       +-----------+


How to run (show_topology/show_switch_status)
---------------------------------------------

1. Change to trema directory and build trema, if you haven't done so already. 

        $ cd $TREMA_HOME
        $ ./build.rb

2. Start network to discover

        $ ./trema run ./objects/examples/dumper/dumper -c src/examples/topology/topology_fullmesh.conf &
 
   # dumper is used in the above example, only because trema cannot run without 
   # any controller specified.

3. Enable link discovery feature
   by adding "--always_run_discovery" option when starting topology

        $ ./trema run ./objects/examples/topology/topology -d --always_run_discovery

   Or by using API call.

        $ ./trema run ./objects/examples/topology/enable_discovery

4. (C version) Run show_topology/show_switch_status from trema run 

        $ ./trema run objects/examples/topology/show_topology
        $ ./trema run objects/examples/topology/show_switch_status

   Or invoke each commands directly

        $ env TREMA_HOME=`pwd` src/examples/topology/show_topology
        $ env TREMA_HOME=`pwd` src/examples/topology/show_switch_status

4. (Ruby version) Run show-topology.rb/show-switch-status.rb from trema run 

        $ ./trema run src/examples/topology/show-topology.rb
        $ ./trema run src/examples/topology/show-switch-status.rb



License & Terms
---------------

Copyright (C) 2008-2013 NEC Corporation

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
