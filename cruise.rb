#!/usr/bin/env ruby
#
# Unit test & acceptance test runner for Trema.
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


#
# Guard Trema codebase's coverage from dropping below the following
# threshold.
#

$coverage_threshold = 70.2


################################################################################
# Load libraries
################################################################################

$LOAD_PATH.unshift( File.expand_path( File.dirname( __FILE__ ) + "/ruby" ) )

require "rubygems"

require "English"
require "blocker"
require "fileutils"
require "find"
require "optparse"
require "stringio"
require "sub-process"
require "trema/path"


include FileUtils


################################################################################
# Helper methods
################################################################################

def path_string
  paths = ENV[ "PATH" ].split( ":" )
  paths << Gem::bindir unless paths.include?( Gem::bindir )
  paths.join ":"
end


def sh cmd
  ENV[ "LC_ALL" ] = "C"
  ENV[ "PATH" ] = path_string
  puts cmd if $verbose
  unless system( cmd )
    raise "Command '#{ cmd }' failed!"
  end
end


################################################################################
# Code coverage (gcov)
################################################################################

$c_files = {}


class Testee
  attr_reader :coverage
  attr_writer :lines


  def initialize path
    @path = path
    @lines = 0
    @coverage = 0.0
  end


  def name
    File.basename( @path )
  end


  def lines
    return @lines if @lines != 0

    n = 0
    in_comment = false
    File.open( @path, "r" ).each_line do | l |
      next if /^\s+\/\//=~ l # comments starting with //
      next if /^\s*$/=~ l # empty lines
      next if /^\s*\/\*.*\*\//=~l # /* ... */
      if /^\s*\/\*/=~ l # /*
        in_comment = true
        next
      end
      if /^\s*\*\//=~ l # */
        in_comment = false
        next
      end
      next if in_comment
      n += 1
    end
    n
  end


  def lines_tested
    @lines * @coverage / 100.0
  end


  def coverage= value
    if value > @coverage
      @coverage = value
    end
  end


  def <=> other
    @name <=> other.name
  end
end


def testees
  $c_files.delete_if do | key, value |
    File.basename( key ) == "trema_wrapper.c"
  end
end


def files_tested
  testees.values.select do | each |
    each.coverage != 0.0
  end
end


def files_not_tested
  testees.values - files_tested
end


def lines_total
  testees.values.inject( 0 ) do | r, each |
    r += each.lines
  end
end


def lines_tested
  testees.values.inject( 0 ) do | r, each |
    r += each.lines_tested
  end
end


def coverage
  sprintf( "%3.1f", lines_tested / lines_total * 100.0 ).to_f
end


def diff_coverage_threshold
  sprintf( "%3.1f", ( $coverage_threshold - coverage ).abs ).to_f
end


def gcov gcda, dir
  file = nil

  cmd = "gcov #{ gcda } -o #{ dir } -n"
  SubProcess.create do | shell |
    shell.on_stdout do | l |
      file = File.expand_path( $1 ) if /^File '(.*)'/=~ l
      testee = $c_files[ file ]
      if /^Lines executed:(.*)% of (.*)$/=~ l and not testee.nil?
        testee.coverage = $1.to_f
        testee.lines = $2.to_i
      end
    end
    shell.on_stderr do | l |
      $stderr.puts l
    end
    shell.on_failure do
      raise "'#{ cmd }' failed."
    end
    shell.exec cmd
  end
end


def measure_coverage
  Find.find( "src/lib", "src/packetin_filter", "src/switch_manager", "src/tremashark", "unittests" ) do | f |
    if /\.c$/=~ f
      path = File.expand_path( f )
      $c_files[ path ] = Testee.new( path )
    end
  end
  Dir.glob( "unittests/objects/*.gcda" ).each do | each |
    gcov each, "unittests/objects"
  end
  Dir.glob( "objects/unittests/*.gcda" ).each do | each |
    gcov each, "objects/unittests"
  end
end


################################################################################
# Summaries
################################################################################

def banner message
  return <<-EOF
#{ message }
================================================================================
EOF
end


def coverage_ranking
  summary = StringIO.new
  testees.values.sort_by do | each |
    each.coverage
  end.each do | each |
    summary.printf "  %45s (%4s LoC): %5.1f%%\n", each.name, each.lines, each.coverage
  end
  summary.string
end


def coverage_threshold_error
  return <<-EOF
#{ banner( "ERROR" ) }
Oops...
Overall coverage DECREASED to #{ coverage }% !!!

IMPROVE the ratio and re-run #{ $0 }.
EOF
end


def update_coverage_threshold_message
  return <<-EOF
#{ banner( "WARNING" ) }
Congratulations !
Overall coverage INCREASED to #{ coverage }% !

Update the $coverage_threshold line at the top of #{ $0 }:

  $coverage_threshold = #{ coverage }
EOF
end


def delta_coverage_threshold
  0.1
end


def show_summary
  puts <<-EOF


#{ banner( "Coverage details" ) }
#{ coverage_ranking }

#{ banner( "Summary" ) }
- Total execution time = #{ ( Time.now - $start_time ).to_i } seconds
- Overall coverage = #{ coverage }% (#{ files_not_tested.size }/#{ testees.size } files not yet tested)


EOF

  if ( coverage < $coverage_threshold ) and ( diff_coverage_threshold > delta_coverage_threshold )
    puts coverage_threshold_error
    puts; puts
    exit -1
  elsif ( coverage > $coverage_threshold ) and ( diff_coverage_threshold > delta_coverage_threshold )
    puts update_coverage_threshold_message
    puts; puts
    exit -1
  end
end


################################################################################
# Tests
################################################################################

def test message
  puts message
  cd Trema.home do
    sh "./build.rb clean"
    begin
      yield
    ensure
      sh "./trema killall"
    end
  end
end


def run_unit_test
  test "Running unit tests ..." do
    sh "./build.rb unittests"
    sh "rake spec"
  end
  measure_coverage
end


def run_acceptance_test
  test "Running acceptance tests ..." do
    sh "rake features"
  end
end


################################################################################
# Main
################################################################################

$options = OptionParser.new

$options.on( "-u", "--unit-test-only" ) do
  $unit_test_only = true
end

$options.on( "-a", "--acceptance-test-only" ) do
  $acceptance_test_only = true
end

$options.separator ""

$options.on( "-h", "--help" ) do
  puts $options.to_s
  exit
end

$options.on( "-v", "--verbose" ) do
  $verbose = true
end

$options.parse! ARGV


def init_cruise
  sh "bundle"
  sh "rake setup"
end


Blocker.start do
  $start_time = Time.now
  cd Trema.home do
    init_cruise
    run_unit_test if not $acceptance_test_only
    run_acceptance_test if not $unit_test_only
    show_summary if not $acceptance_test_only
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
