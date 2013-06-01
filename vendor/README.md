Notes on Third Party Software Packages
======================================

Software packages found in this directory are not part of Trema. Each software
packages is distributed under the terms and conditions described in the package.


Notes on oflops (vendor/oflops)
-------------------------------

oflops is a set of tools for measuring the performance of OpenFlow switches and
controllers. You can find detailed information about oflops on:

  http://www.openflow.org/wk/index.php/Oflops

Since it depends on Net-SNMP but we do not need SNMP related functions in terms
of this OpenFlow controller platform, we disabled the functions. In addition,
because there are some issues on OpenFlow messages that cbench (a performance
measurement tool for OpenFlow controllers) sends, we fixed them. Any changes
from the repository of oflops can be found in oflops_no_snmp+1.0.0.patch.

Notes on Open vSwitch (vendor/openvswitch)
------------------------------------------

In the recent glibc, we need to link librt to use POSIX timer functions
such as "timer_create" or "timer_settime". To use openvswitch-1.2.2
on distributions with the recent glibc, we modified "configure" to
search librt and Open vSwitch tarball distributed with Trema contains
the fix. The change can be found in openvswitch-1.2.2_librt-check.diff.
