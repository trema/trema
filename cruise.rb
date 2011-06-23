#!/usr/bin/env ruby
#
# Unit test & acceptance test runner for Trema.
#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
#
# Copyright (C) 2008-2011 NEC Corporation
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

$coverage_threshold = 95.1


################################################################################
# Load libraries
################################################################################

$LOAD_PATH.unshift( File.expand_path( File.dirname( __FILE__ ) + "/ruby" ) )

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
  gem_bin = "/var/lib/gems/1.8/bin"
  paths = ENV[ "PATH" ].split( ":" )
  paths << gem_bin unless paths.include?( gem_bin )
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

$coverage = {}

$lines_total = 0
$lines_tested = 0

$files_all = []
$files_tested = []


def files_not_tested
  $files_all - $files_tested
end


def coverage
  sprintf( "%3.1f", $lines_tested / $lines_total * 100.0 ).to_f
end


def diff_coverage_threshold
  sprintf( "%3.1f", ( $coverage_threshold - coverage ).abs ).to_f
end


def gcov gcda
  file = nil

  cd File.dirname( gcda ) do
    cmd = "gcov #{ File.basename gcda } -p -l -n"
    SubProcess.create do | shell |
      shell.on_stdout do | l |
        file = File.basename( $1 ) if /^File '(.*)'/=~ l
        $files_tested << file
        if /^Lines executed:(.*)% of (.*)$/=~ l
          coverage = $1.to_f
          lines = $2.to_i
          $lines_total += lines
          $lines_tested += lines * coverage / 100.0
          if $coverage[ file ].nil? or ( $coverage[ file ] and coverage > $coverage[ file ] )
            $coverage[ file ] = coverage
          end
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
end


def measure_coverage
  [ "src", "examples" ].each do | each |
    Find.find each do | f |
      $files_all << File.basename( f ) if /\.c$/=~ f
    end
  end
  Dir.glob( "unittests/objects/*.gcda" ).each do | each |
    gcov each
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
  files_not_tested.sort.each do | each |
    summary.printf "  %40s:   0.0%%\n", each
  end
  $coverage.to_a.sort_by do | each |
    each[ 1 ]
  end.each do | each |
    summary.printf "  %40s: %5.1f%%\n", *each
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
- Overall coverage = #{ coverage }% (#{ files_not_tested.size }/#{ $files_all.size } files not yet tested)


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
    sh "./build.rb coverage"
    sh "./build.rb unittests"
    sh "rake spec"
  end
  measure_coverage
end


def run_acceptance_test
  test "Running acceptance tests ..." do
    sh "./build.rb"
    sh "cucumber"
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
  $start_time = Time.now
  sh "./build.rb distclean"
  mkdir_p Trema.log_directory
end


Blocker.start do
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
