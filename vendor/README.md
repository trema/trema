Notes on Third Party Software Packages
======================================

Software packages found in this directory are not part of Trema. Each software
packages is distributed under the terms and conditions described in the package.

Notes on Open vSwitch (vendor/openvswitch)
------------------------------------------

In the recent glibc, we need to link librt to use POSIX timer functions
such as "timer_create" or "timer_settime". To use openvswitch-1.2.2
on distributions with the recent glibc, we modified "configure" to
search librt and Open vSwitch tarball distributed with Trema contains
the fix. The change can be found in openvswitch-1.2.2_librt-check.diff.
