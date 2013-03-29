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
task :build_trema => [ Trema.log, Trema.pid, Trema.sock ] do
  sh "#{ Gem.ruby } ./build.rb"
end


require "paper-house/executable-task"
require "paper-house/ruby-library-task"
require "paper-house/shared-library-task"
require "paper-house/static-library-task"
require "trema/version"


CFLAGS = [
  "-g",
  "-std=gnu99",
  "-D_GNU_SOURCE",
  "-fno-strict-aliasing",
  "-Werror",
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
task "libtrema:gcov" => "vendor:openflow"
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
# Maintenance Tasks
################################################################################

# Generate a monolithic rant file"
# FIXME: Remove dependency to rant
task "build.rb" do
  sh "rant-import --force --auto .mono.rant"
end


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
# Cruise
################################################################################

task :setup do
  sh "./build.rb distclean"
end


################################################################################
# Tests
################################################################################

task :travis => [ :setup, :build_trema ]


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
$quality_targets = if ENV[ "QUALITY_TARGETS" ]
                     ENV[ "QUALITY_TARGETS" ].split
                   else
                     $ruby_sources
                   end


desc "Enforce Ruby code quality with static analysis of code"
task :quality => [ :reek, :flog, :flay ]


begin
  require "reek/rake/task"

  Reek::Rake::Task.new do | t |
    t.fail_on_error = true
    t.verbose = false
    t.ruby_opts = [ "-rubygems" ]
    t.reek_opts = "--quiet"
    t.source_files = $quality_targets
  end
rescue LoadError
  $stderr.puts $!.to_s
end

begin
  require "flog"

  desc "Analyze for code complexity"
  task :flog do
    flog = Flog.new( :continue => true )
    flog.flog $quality_targets
    threshold = 10

    bad_methods = flog.totals.select do | name, score |
      name != "main#none" && score > threshold
    end
    bad_methods.sort do | a, b |
      a[ 1 ] <=> b[ 1 ]
    end.each do | name, score |
      puts "%8.1f: %s" % [ score, name ]
    end
    unless bad_methods.empty?
      raise "#{ bad_methods.size } methods have a flog complexity > #{ threshold }"
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


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
