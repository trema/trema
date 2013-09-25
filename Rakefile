#
# Copyright (C) 2008-2013 NEC Corporation
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2, as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#


$LOAD_PATH.unshift( File.expand_path( File.dirname( __FILE__ ) + "/ruby" ) )


require "rubygems"
require "rake"
require "trema/executables"
require "trema/path"


task :default => :build_trema

directory Trema.log
directory Trema.pid
directory Trema.sock

desc "Build Trema"
task :build_trema => [
  Trema.log,
  Trema.pid,
  Trema.sock,
  :management_commands,
  :rubylib,
  :switch_manager,
  :switch_daemon,
  :packetin_filter,
  :tremashark,
  :vendor,
  :examples
]


require "paper_house"
require "trema/version"


CFLAGS = [
  "-g",
  "-std=gnu99",
  "-D_GNU_SOURCE",
  "-fno-strict-aliasing",
  "-Wall",
  "-Wextra",
  "-Wformat=2",
  "-Wcast-qual",
  "-Wcast-align",
  "-Wwrite-strings",
  "-Wconversion",
  "-Wfloat-equal",
  "-Wpointer-arith",
]
CFLAGS << "-Werror" if RUBY_VERSION < "1.9.0"


desc "Build Trema C library (static library)."
task "libtrema:static" => "vendor:openflow"
PaperHouse::StaticLibraryTask.new "libtrema:static" do | task |
  task.library_name = "libtrema"
  task.target_directory = Trema.lib
  task.sources = "#{ Trema.include }/*.c"
  task.cflags = CFLAGS
  task.includes = [ Trema.openflow ]
end


desc "Build Trema C library (coverage)."
task "libtrema:gcov" => [ "vendor:openflow" ]
PaperHouse::StaticLibraryTask.new "libtrema:gcov" do | task |
  task.library_name = "libtrema"
  task.target_directory = "#{ Trema.home }/objects/unittests"
  task.sources = "#{ Trema.include }/*.c"
  task.includes = [ Trema.openflow ]
  task.cflags = [ "--coverage" ] + CFLAGS
end


desc "Build Trema C library (shared library)."
task "libtrema:shared" => "vendor:openflow"
PaperHouse::SharedLibraryTask.new "libtrema:shared" do | task |
  task.library_name = "libtrema"
  task.target_directory = Trema.lib
  task.version = Trema::VERSION
  task.sources = "#{ Trema.include }/*.c"
  task.includes = [ Trema.openflow ]
  task.cflags = CFLAGS
end


desc "Build Trema Ruby library."
task "rubylib" => "libtrema:static"
PaperHouse::RubyLibraryTask.new "rubylib" do | task |
  task.library_name = "trema"
  task.target_directory = Trema.ruby
  task.sources = "#{ Trema.ruby }/trema/*.c"
  task.includes = [ Trema.include, Trema.openflow ]
  task.cflags = CFLAGS
  task.ldflags = [ "-Wl,-Bsymbolic", "-L#{ Trema.lib }" ]
  task.library_dependencies = [
    "trema",
    "sqlite3",
    "pthread",
    "rt",
    "dl",
    "crypt",
    "m"
  ]
end


desc "Build switch manager."
task :switch_manager => "libtrema:static"

PaperHouse::ExecutableTask.new :switch_manager do | task |
  task.target_directory = File.dirname( Trema::Executables.switch_manager )
  task.sources = [
    "src/switch_manager/dpid_table.c",
    "src/switch_manager/event_forward_entry_manipulation.c",
    "src/switch_manager/secure_channel_listener.c",
    "src/switch_manager/switch_manager.c",
    "src/switch_manager/switch_option.c",
  ]
  task.includes = [ Trema.include, Trema.openflow ]
  task.cflags = CFLAGS
  task.ldflags = "-L#{ Trema.lib }"
  task.library_dependencies = [
    "trema",
    "sqlite3",
    "pthread",
    "rt",
    "dl",
  ]
end


desc "Build switch daemon."
task :switch_daemon => "libtrema:static"

PaperHouse::ExecutableTask.new :switch_daemon do | task |
  task.executable_name = File.basename( Trema::Executables.switch )
  task.target_directory = File.dirname( Trema::Executables.switch )
  task.sources = [
    "src/switch_manager/cookie_table.c",
    "src/switch_manager/event_forward_entry_manipulation.c",
    "src/switch_manager/ofpmsg_recv.c",
    "src/switch_manager/ofpmsg_send.c",
    "src/switch_manager/secure_channel_receiver.c",
    "src/switch_manager/secure_channel_sender.c",
    "src/switch_manager/service_interface.c",
    "src/switch_manager/switch.c",
    "src/switch_manager/switch_option.c",
    "src/switch_manager/xid_table.c",
  ]
  task.includes = [ Trema.include, Trema.openflow ]
  task.cflags = CFLAGS
  task.ldflags = "-L#{ Trema.lib }"
  task.library_dependencies = [
    "trema",
    "sqlite3",
    "pthread",
    "rt",
    "dl",
  ]
end


################################################################################
# Extract OpenFlow reference implementation
################################################################################

task "vendor:openflow" => Trema.openflow_h
file Trema.openflow_h => Trema.objects do
  sh "tar xzf #{ Trema.vendor_openflow }.tar.gz -C #{ Trema.vendor }"
  cp_r "#{ Trema.vendor_openflow }/include/openflow", Trema.objects
end
directory Trema.objects

CLOBBER.include File.join( Trema.objects, "openflow" )
CLOBBER.include File.join( Trema.vendor_openflow )


################################################################################
# Build phost
################################################################################

task "vendor:phost" => [ Trema::Executables.phost, Trema::Executables.cli ]

def phost_src
  File.join Trema.vendor_phost, "src"
end

file Trema::Executables.phost do
  cd phost_src do
    sh "make"
  end
  mkdir_p File.dirname( Trema::Executables.phost )
  install File.join( phost_src, "phost" ), Trema::Executables.phost, :mode => 0755
end

file Trema::Executables.cli do
  cd phost_src do
    sh "make"
  end
  mkdir_p File.dirname( Trema::Executables.cli )
  install File.join( phost_src, "cli" ), Trema::Executables.cli, :mode => 0755
end

CLEAN.include FileList[ File.join( phost_src, "*.o" ) ]
CLEAN.include File.join( phost_src, "phost" )
CLEAN.include File.join( phost_src, "cli" )
CLOBBER.include Trema.phost


################################################################################
# Build vendor/*
################################################################################

task :vendor => [
  "vendor:oflops",
  "vendor:openflow",
  "vendor:openvswitch",
  "vendor:phost",
]


################################################################################
# Build Open vSwitch
################################################################################

task "vendor:openvswitch" => Trema::Executables.ovs_openflowd
file Trema::Executables.ovs_openflowd do
  sh "tar xzf #{ Trema.vendor_openvswitch }.tar.gz -C #{ Trema.vendor }"
  cd Trema.vendor_openvswitch do
    sh "./configure --prefix=#{ Trema.openvswitch } --with-rundir=#{ Trema.sock }"
    sh "make install"
    cp "./tests/test-openflowd", Trema::Executables.ovs_openflowd
  end
end

CLEAN.include Trema.vendor_openvswitch
CLOBBER.include Trema.openvswitch


################################################################################
# Build packetin filter
################################################################################

desc "Build packetin filter."
task :packetin_filter => "libtrema:static"

PaperHouse::ExecutableTask.new :packetin_filter do | task |
  task.executable_name = File.basename( Trema::Executables.packetin_filter )
  task.target_directory = File.dirname( Trema::Executables.packetin_filter )
  task.sources = [ "src/packetin_filter/*.c" ]
  task.includes = [ Trema.include, Trema.openflow ]
  task.cflags = CFLAGS
  task.ldflags = "-L#{ Trema.lib }"
  task.library_dependencies = [
                               "trema",
                               "sqlite3",
                               "pthread",
                               "rt",
                               "dl",
                              ]
end


################################################################################
# Build oflops
################################################################################

def cbench_command
  File.join Trema.objects, "oflops/bin/cbench"
end

task "vendor:oflops" => cbench_command
file cbench_command => Trema.openflow_h do
  sh "tar xzf #{ Trema.vendor_oflops }.tar.gz -C #{ Trema.vendor }"
  cd Trema.vendor_oflops do
    sh "./configure --prefix=#{ Trema.oflops } --with-openflow-src-dir=#{ Trema.vendor_openflow }"
    sh "make install"
  end
end

CLEAN.include Trema.oflops
CLOBBER.include Trema.vendor_oflops


################################################################################
# cmockery
################################################################################

task "vendor:cmockery" => Trema.libcmockery_a
file Trema.libcmockery_a do
  sh "tar xzf #{ Trema.vendor_cmockery }.tar.gz -C #{ Trema.vendor }"
  cd Trema.vendor_cmockery do
    sh "./configure --prefix=#{ Trema.cmockery }"
    sh "make install"
  end
end

CLEAN.include Trema.vendor_cmockery
CLOBBER.include Trema.cmockery


################################################################################
# Build examples
################################################################################

$standalone_examples = [
                        "cbench_switch",
                        "dumper",
                        "learning_switch",
                        "list_switches",
                        "multi_learning_switch",
                        "packet_in",
                        "repeater_hub",
                        "switch_info",
                        "switch_monitor",
                        "traffic_monitor",
                       ]

desc "Build examples."
task :examples =>
  $standalone_examples.map { | each | "examples:#{ each }" } +
  [
   "examples:openflow_switch",
   "examples:openflow_message",
   "examples:switch_event_config",
   "examples:packetin_filter_config"
  ]

$standalone_examples.each do | each |
  name = "examples:#{ each }"

  task name => "libtrema:static"
  PaperHouse::ExecutableTask.new name do | task |
    task.executable_name = each
    task.target_directory = File.join( Trema.objects, "examples", each )
    task.sources = [ "src/examples/#{ each }/*.c" ]
    task.includes = [ Trema.include, Trema.openflow ]
    task.cflags = CFLAGS
    task.ldflags = "-L#{ Trema.lib }"
    task.library_dependencies = [
                                 "trema",
                                 "sqlite3",
                                 "pthread",
                                 "rt",
                                 "dl",
                                ]
  end
end


################################################################################
# Build openflow switches
################################################################################

$openflow_switches = [
                      "hello_switch",
                      "echo_switch",
                     ]

task "examples:openflow_switch" => $openflow_switches.map { | each | "examples:openflow_switch:#{ each }" }

$openflow_switches.each do | each |
  name = "examples:openflow_switch:#{ each }"

  task name => "libtrema:static"
  PaperHouse::ExecutableTask.new name do | task |
    task.executable_name = each
    task.target_directory = File.join( Trema.objects, "examples", "openflow_switch" )
    task.sources = [ "src/examples/openflow_switch/#{ each }.c" ]
    task.includes = [ Trema.include, Trema.openflow ]
    task.cflags = CFLAGS
    task.ldflags = "-L#{ Trema.lib }"
    task.library_dependencies = [
                                 "trema",
                                 "sqlite3",
                                 "pthread",
                                 "rt",
                                 "dl",
                                ]
  end
end


################################################################################
# Build openflow messages
################################################################################

$openflow_messages = [
                      "echo",
                      "features_request",
                      "hello",
                      "set_config",
                      "vendor_action",
                     ]

task "examples:openflow_message" => $openflow_messages.map { | each | "examples:openflow_message:#{ each }" }

$openflow_messages.each do | each |
  name = "examples:openflow_message:#{ each }"

  task name => "libtrema:static"
  PaperHouse::ExecutableTask.new name do | task |
    task.executable_name = each
    task.target_directory = File.join( Trema.objects, "examples", "openflow_message" )
    task.sources = [ "src/examples/openflow_message/#{ each }.c" ]
    task.includes = [ Trema.include, Trema.openflow ]
    task.cflags = CFLAGS
    task.ldflags = "-L#{ Trema.lib }"
    task.library_dependencies = [
                                 "trema",
                                 "sqlite3",
                                 "pthread",
                                 "rt",
                                 "dl",
                                ]
  end
end


###############################################################################
# Build switch_event_config
###############################################################################

$switch_event_config = [
                        "add_forward_entry",
                        "delete_forward_entry",
                        "set_forward_entries",
                        "dump_forward_entries",
                       ]

task "examples:switch_event_config" => $switch_event_config.map { | each | "examples:switch_event_config:#{ each }" }

$switch_event_config.each do | each |
  name = "examples:switch_event_config:#{ each }"

  task name => "libtrema:static"
  PaperHouse::ExecutableTask.new name do | task |
    task.executable_name = each
    task.target_directory = File.join( Trema.objects, "examples", "switch_event_config" )
    task.sources = [ "src/examples/switch_event_config/#{ each }.c" ]
    task.includes = [ Trema.include, Trema.openflow ]
    task.cflags = CFLAGS
    task.ldflags = "-L#{ Trema.lib }"
    task.library_dependencies = [
                                 "trema",
                                 "sqlite3",
                                 "pthread",
                                 "rt",
                                 "dl",
                                ]
  end
end


################################################################################
# Build packetin_filter_config
################################################################################

$packetin_filter_config = [
                           "add_filter",
                           "delete_filter",
                           "delete_filter_strict",
                           "dump_filter",
                           "dump_filter_strict",
                          ]

task "examples:packetin_filter_config" => $packetin_filter_config.map { | each | "examples:packetin_filter_config:#{ each }" }

$packetin_filter_config.each do | each |
  name = "examples:packetin_filter_config:#{ each }"

  task name => "libtrema:static"
  PaperHouse::ExecutableTask.new name do | task |
    task.executable_name = each
    task.target_directory = File.join( Trema.objects, "examples", "packetin_filter_config" )
    task.sources = [ "src/examples/packetin_filter_config/#{ each }.c", "src/examples/packetin_filter_config/utils.c"]
    task.includes = [ Trema.include, Trema.openflow ]
    task.cflags = CFLAGS
    task.ldflags = "-L#{ Trema.lib }"
    task.library_dependencies = [
                                 "trema",
                                 "sqlite3",
                                 "pthread",
                                 "rt",
                                 "dl",
                                ]
  end
end


################################################################################
# Run cbench benchmarks
################################################################################

def cbench_command
  File.join Trema.objects, "oflops/bin/cbench"
end


def cbench_latency_mode_options
  "--switches 1 --loops 10 --delay 1000"
end


def cbench_throughput_mode_options
  cbench_latency_mode_options + " --throughput"
end


def cbench controller, options
  begin
    sh "#{ controller }"
    sh "#{ cbench_command } #{ options }"
  ensure
    sh "./trema killall"
  end
end


def cbench_c_controller
  "./trema run ./objects/examples/cbench_switch/cbench_switch -d"
end


def cbench_ruby_controller
  "./trema run src/examples/cbench_switch/cbench-switch.rb -d"
end


def run_cbench controller
  cbench controller, cbench_latency_mode_options
  cbench controller, cbench_throughput_mode_options
end


def cbench_profile options
  valgrind = "valgrind --tool=callgrind --trace-children=yes"
  begin
    sh "#{ valgrind } #{ cbench_c_controller }"
    sh "#{ cbench_command } #{ options }"
  ensure
    sh "./trema killall"
  end
end

CLEAN.include FileList[ "callgrind.out.*" ]


desc "Run the c cbench switch controller to benchmark"
task "cbench" => "cbench:ruby"


desc "Run the c cbench switch controller to benchmark"
task "cbench:c" => :default do
  run_cbench cbench_c_controller
end


desc "Run the ruby cbench switch controller to benchmark"
task "cbench:ruby" => :default do
  run_cbench cbench_ruby_controller
end


desc "Run cbench with profiling enabled."
task "cbench:profile" => :default do
  cbench_profile cbench_latency_mode_options
  cbench_profile cbench_throughput_mode_options
end


################################################################################
# Build management commands
################################################################################

$management_commands = [
                       "application",
                       "echo",
                       "set_logging_level",
                       "show_stats",
                      ]

desc "Build management commands."
task :management_commands => $management_commands.map { | each | "management:#{ each }" }

$management_commands.each do | each |
  name = "management:#{ each }"

  task name => "libtrema:static"
  PaperHouse::ExecutableTask.new name do | task |
    task.executable_name = each
    task.target_directory = File.join( Trema.objects, "management" )
    task.sources = [ "src/management/#{ each }.c" ]
    task.includes = [ Trema.include, Trema.openflow ]
    task.cflags = CFLAGS
    task.ldflags = "-L#{ Trema.lib }"
    task.library_dependencies = [
                                 "trema",
                                 "sqlite3",
                                 "pthread",
                                 "rt",
                                 "dl",
                                ]
  end
end


################################################################################
# Tremashark
################################################################################

desc "Build tremashark."
task :tremashark => [ :packet_capture, :syslog_relay, :stdin_relay, :openflow_wireshark_plugin, "libtrema:static" ]

PaperHouse::ExecutableTask.new :tremashark do | task |
  task.executable_name = File.basename( Trema::Executables.tremashark )
  task.target_directory = File.dirname( Trema::Executables.tremashark )
  task.sources = [
                  "src/tremashark/pcap_queue.c",
                  "src/tremashark/queue.c",
                  "src/tremashark/tremashark.c",
                 ]
  task.includes = [ Trema.include, Trema.openflow ]
  task.cflags = CFLAGS
  task.ldflags = "-L#{ Trema.lib }"
  task.library_dependencies = [
                               "trema",
                               "sqlite3",
                               "pthread",
                               "rt",
                               "dl",
                               "pcap"
                              ]
end


task :packet_capture => "libtrema:static"

PaperHouse::ExecutableTask.new :packet_capture do | task |
  task.executable_name = File.basename( Trema::Executables.packet_capture )
  task.target_directory = File.dirname( Trema::Executables.packet_capture )
  task.sources = [
                  "src/tremashark/packet_capture.c",
                  "src/tremashark/queue.c",
                 ]
  task.includes = [ Trema.include, Trema.openflow ]
  task.cflags = CFLAGS
  task.ldflags = "-L#{ Trema.lib }"
  task.library_dependencies = [
                               "trema",
                               "sqlite3",
                               "pthread",
                               "rt",
                               "dl",
                               "pcap"
                              ]
end


task :syslog_relay => "libtrema:static"

PaperHouse::ExecutableTask.new :syslog_relay do | task |
  task.executable_name = File.basename( Trema::Executables.syslog_relay )
  task.target_directory = File.dirname( Trema::Executables.syslog_relay )
  task.sources = [ "src/tremashark/syslog_relay.c" ]
  task.includes = [ Trema.include, Trema.openflow ]
  task.cflags = CFLAGS
  task.ldflags = "-L#{ Trema.lib }"
  task.library_dependencies = [
                               "trema",
                               "sqlite3",
                               "pthread",
                               "rt",
                               "dl",
                               "pcap"
                              ]
end


task :stdin_relay => "libtrema:static"

PaperHouse::ExecutableTask.new :stdin_relay do | task |
  task.executable_name = File.basename( Trema::Executables.stdin_relay )
  task.target_directory = File.dirname( Trema::Executables.stdin_relay )
  task.sources = [ "src/tremashark/stdin_relay.c" ]
  task.includes = [ Trema.include, Trema.openflow ]
  task.cflags = CFLAGS
  task.ldflags = "-L#{ Trema.lib }"
  task.library_dependencies = [
                               "trema",
                               "sqlite3",
                               "pthread",
                               "rt",
                               "dl",
                               "pcap"
                              ]
end


$packet_openflow_so = File.join( Trema.vendor_openflow_git, "utilities", "wireshark_dissectors", "openflow", "packet-openflow.so" )
$wireshark_plugins_dir = File.join( File.expand_path( "~" ), ".wireshark", "plugins" )
$wireshark_plugin = File.join( $wireshark_plugins_dir, File.basename( $packet_openflow_so ) )

file $packet_openflow_so do
  sh "tar xzf #{ Trema.vendor_openflow_git }.tar.gz -C #{ Trema.vendor }"
  cd File.dirname( $packet_openflow_so ) do
    sh "make"
  end
end

file $wireshark_plugin => [ $packet_openflow_so, $wireshark_plugins_dir ] do
  cp $packet_openflow_so, $wireshark_plugins_dir
end

directory $wireshark_plugins_dir

task :openflow_wireshark_plugin => $wireshark_plugin

CLEAN.include Trema.vendor_openflow_git


################################################################################
# Maintenance Tasks
################################################################################

begin
  require "bundler/gem_tasks"
rescue LoadError
  $stderr.puts $!.to_s
end


################################################################################
# Relish
################################################################################

task :relish do
  sh "relish push trema/trema"
end


################################################################################
# C Unit tests.
################################################################################

def libtrema_unit_tests
  {
    :byteorder_test => [ :log, :utility, :wrapper, :trema_wrapper ],
    :daemon_test => [ :log, :utility, :wrapper, :trema_wrapper ],
    :ether_test => [ :buffer, :log, :utility, :wrapper, :trema_wrapper ],
    :messenger_test => [ :doubly_linked_list, :hash_table, :event_handler, :linked_list, :utility, :wrapper, :timer, :log, :trema_wrapper ],
    :openflow_application_interface_test => [ :buffer, :byteorder, :hash_table, :doubly_linked_list, :linked_list, :log, :openflow_message, :packet_info, :stat, :trema_wrapper, :utility, :wrapper ],
    :openflow_message_test => [ :buffer, :byteorder, :linked_list, :log, :packet_info, :utility, :wrapper, :trema_wrapper ],
    :packet_info_test => [ :buffer, :log, :utility, :wrapper, :trema_wrapper ],
    :stat_test => [ :hash_table, :doubly_linked_list, :log, :utility, :wrapper, :trema_wrapper ],
    :timer_test => [ :log, :utility, :wrapper, :doubly_linked_list, :trema_wrapper ],
    :trema_test => [ :utility, :log, :wrapper, :doubly_linked_list, :trema_private, :trema_wrapper ],
  }
end


def test_c_files test
  names = [ test.to_s.gsub( /_test$/, "" ) ] + libtrema_unit_tests[ test ]
  names.collect do | each |
    if each == :buffer
      [ "src/lib/buffer.c", "unittests/buffer_stubs.c" ]
    elsif each == :wrapper
      [ "src/lib/wrapper.c", "unittests/wrapper_stubs.c" ]
    else
      "src/lib/#{ each }.c"
    end
  end.flatten
end


directory "objects/unittests"

task :build_old_unittests => libtrema_unit_tests.keys.map { | each | "unittests:#{ each }" }

libtrema_unit_tests.keys.each do | each |
  PaperHouse::ExecutableTask.new "unittests:#{ each }" do | task |
    name = "unittests:#{ each }"
    task name => [ "vendor:cmockery", "vendor:openflow", "objects/unittests" ]

    task.executable_name = each.to_s
    task.target_directory = File.join( Trema.home, "unittests/objects" )
    task.sources = test_c_files( each ) + [ "unittests/lib/#{ each }.c" ]
    task.includes = [ Trema.include, Trema.openflow, File.dirname( Trema.cmockery_h ), "unittests" ]
    task.cflags = [ "-DUNIT_TESTING", "--coverage", CFLAGS ]
    task.ldflags = "-DUNIT_TESTING -L#{ File.dirname Trema.libcmockery_a } --coverage --static"
    task.library_dependencies = [
                                 "cmockery",
                                 "sqlite3",
                                 "pthread",
                                 "rt",
                                 "dl",
                                 "pcap"
                                ]
  end
end


# new unittest
$tests = [
          "objects/unittests/buffer_test",
          "objects/unittests/doubly_linked_list_test",
          "objects/unittests/ether_test",
          "objects/unittests/event_forward_interface_test",
          "objects/unittests/hash_table_test",
          "objects/unittests/linked_list_test",
          "objects/unittests/log_test",
          "objects/unittests/packetin_filter_interface_test",
          "objects/unittests/packet_info_test",
          "objects/unittests/packet_parser_test",
          "objects/unittests/persistent_storage_test",
          "objects/unittests/trema_private_test",
          "objects/unittests/utility_test",
          "objects/unittests/wrapper_test",
          "objects/unittests/match_table_test",
          "objects/unittests/message_queue_test",
          "objects/unittests/management_interface_test",
          "objects/unittests/management_service_interface_test",
         ]

task :build_unittests => $tests.map { | each | "unittests:" + File.basename( each ) }

$tests.each do | _each |
  each = File.basename( _each )

  task "unittests:#{ each }" => [ "libtrema:gcov", "vendor:cmockery" ]
  PaperHouse::ExecutableTask.new "unittests:#{ each }" do | task |
    task.executable_name = each.to_s
    task.target_directory = File.join( Trema.home, "unittests/objects" )
    task.sources = [ "unittests/lib/#{ each }.c", "unittests/cmockery_trema.c" ]
    task.includes = [ Trema.include, Trema.openflow, File.dirname( Trema.cmockery_h ), "unittests" ]
    task.cflags = [ "--coverage", CFLAGS ]
    task.ldflags = "-L#{ File.dirname Trema.libcmockery_a } -Lobjects/unittests --coverage --static"
    task.library_dependencies = [
                                 "trema",
                                 "cmockery",
                                 "sqlite3",
                                 "pthread",
                                 "rt",
                                 "dl",
                                ]
  end
end


desc "Run unittests"
task :unittests => [ :build_old_unittests, :build_unittests ] do
  Dir.glob( "unittests/objects/*_test" ).each do | each |
    puts "Running #{ each }..."
    sh each
  end
end


################################################################################
# Tests
################################################################################

task :travis => [ :clobber, :build_trema, "spec:travis" ]


begin
  require "rspec/core"
  require "rspec/core/rake_task"

  task :spec => :build_trema
  RSpec::Core::RakeTask.new do | task |
    task.verbose = $trace
    task.pattern = FileList[ "spec/**/*_spec.rb" ]
    task.rspec_opts = "--format documentation --color"
  end

  task "spec:actions" => :build_trema
  RSpec::Core::RakeTask.new( "spec:actions" ) do | task |
    task.verbose = $trace
    task.pattern = FileList[ "spec/**/*_spec.rb" ]
    task.rspec_opts = "--tag type:actions --format documentation --color"
  end


  task "spec:travis" => :build_trema
  RSpec::Core::RakeTask.new( "spec:travis" ) do | task |
    task.verbose = $trace
    task.pattern = FileList[ "spec/trema/hello_spec.rb", "spec/trema/echo-*_spec.rb" ]
    task.rspec_opts = "--tag ~sudo --format documentation --color"
  end


  task :rcov => :build_trema
  RSpec::Core::RakeTask.new( :rcov ) do | spec |
    spec.pattern = "spec/**/*_spec.rb"
    spec.rcov = true
    spec.rcov_opts = [ "-x", "gems" ]
  end
rescue LoadError
  $stderr.puts $!.to_s
end


begin
  require "cucumber/rake/task"
  task :features => :build_trema
  Cucumber::Rake::Task.new( :features ) do | t |
    t.cucumber_opts = "features --tags ~@wip"
  end
rescue LoadError
  $stderr.puts $!.to_s
end


################################################################################
# Code Quality Tasks
################################################################################

$ruby_sources = FileList[ "ruby/**/*.rb", "src/**/*.rb" ]


desc "Enforce Ruby code quality with static analysis of code"
task :quality => [ :reek, :flog, :flay ]


begin
  require "reek/rake/task"

  Reek::Rake::Task.new do | t |
    t.fail_on_error = false
    t.verbose = false
    t.ruby_opts = [ "-rubygems" ]
    t.reek_opts = "--quiet"
    t.source_files = $ruby_sources
  end
rescue LoadError
  $stderr.puts $!.to_s
end


begin
  require "flog"

  desc "Analyze for code complexity"
  task :flog do
    flog = Flog.new( :continue => true )
    flog.flog *$ruby_sources
    threshold = 10

    bad_methods = flog.totals.select do | name, score |
      ( not ( /##{flog.no_method}$/=~ name ) ) and score > threshold
    end
    bad_methods.sort do | a, b |
      a[ 1 ] <=> b[ 1 ]
    end.reverse.each do | name, score |
      puts "%8.1f: %s" % [ score, name ]
    end
    unless bad_methods.empty?
      $stderr.puts "#{ bad_methods.size } methods have a flog complexity > #{ threshold }"
    end
  end
rescue LoadError
  $stderr.puts $!.to_s
end


begin
  require "flay"
  require "flay_task"

  FlayTask.new do | t |
    t.dirs = $ruby_sources.collect do | each |
      each[ /[^\/]+/ ]
    end.uniq
    t.threshold = 0
    t.verbose = true
  end
rescue LoadError
  $stderr.puts $!.to_s
end


################################################################################
# YARD
################################################################################

begin
  require "yard"

  YARD::Rake::YardocTask.new do | t |
   t.files = [ "ruby/trema/**/*.c", "ruby/trema/**/*.rb" ]
   t.options = [ "--no-private" ]
   t.options << "--debug" << "--verbose" if $trace
  end
rescue LoadError
  $stderr.puts $!.to_s
end


################################################################################
# TODO, FIXME etc.
################################################################################

desc "Print list of notes."
task :notes do
  keywords = [ "TODO", "FIXME", "XXX" ]
  keywords.each do | each |
    system "find src unittests -name '*.c' | xargs grep -n #{ each }"
    system "find ruby spec features -name '*.rb' | xargs grep -n #{ each }"
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
