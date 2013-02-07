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


require "bundler"
begin
  Bundler.setup :default, :development
rescue Bundler::BundlerError => e
  $stderr.puts e.message
  $stderr.puts "Run `bundle install` to install missing gems"
  exit e.status_code
end


$LOAD_PATH.unshift( File.expand_path( File.dirname( __FILE__ ) + "/../../ruby" ) )


require "aruba/cucumber"
require "rspec"
require "tempfile"
require "trema/executables"
require "trema/monkey-patch/string"


def run command
  raise "Failed to execute #{ command }" unless system( command )
end


def ps_entry_of name
  `ps -ef | grep -w "#{ name } " | grep -v grep`
end


def cucumber_log name
  File.join Trema.log, name
end


def new_tmp_log
  system "rm #{ Trema.log }/tmp.*" # cleanup
  `mktemp --tmpdir=#{ Trema.log }`.chomp
end


# show_stats output format:
# ip_dst,tp_dst,ip_src,tp_src,n_pkts,n_octets
def count_packets stats
  return 0 if stats.split.size <= 1
  stats.split[ 1..-1 ].inject( 0 ) do | sum, each |
    sum += each.split( "," )[ 4 ].to_i
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
