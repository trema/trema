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
require "trema/path"

task :default => :build

directory Trema.log
directory Trema.pid
directory Trema.sock

task :build => [ Trema.log, Trema.pid, Trema.sock ] do
  sh "#{ Gem.ruby } ./build.rb"
end


################################################################################
# Maintenance Tasks
################################################################################

# FIXME: Remove dependency to rant
desc "Generate a monolithic rant file"
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
  sh "bundle update"
  sh "bundle install"
end


################################################################################
# Tests
################################################################################

task :travis => [ :setup, :build, :spec ]


begin
  require "rspec/core"
  require "rspec/core/rake_task"

  task :spec => [ :build ]
  RSpec::Core::RakeTask.new do | task |
    task.verbose = $trace
    task.pattern = FileList[ "spec/**/*_spec.rb" ]
    task.rspec_opts = "--format documentation --color"
  end

  task "spec:actions" => [ :build ]
  RSpec::Core::RakeTask.new( "spec:actions" ) do | task |
    task.verbose = $trace
    task.pattern = FileList[ "spec/**/*_spec.rb" ]
    task.rspec_opts = "--tag type:actions --format documentation --color"
  end

  task :rcov => [ :build ]
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
  task :features => [ :build ]
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
