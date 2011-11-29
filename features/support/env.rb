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


$LOAD_PATH.unshift( File.expand_path( File.dirname( __FILE__ ) + "/../../ruby" ) )


require "rspec"
require "tempfile"
require "trema/executables"


class String
  def camelize
    self.split( /[^a-z0-9]/i ).map do | each |
      each.capitalize
    end.join
  end
end


def run command
  raise "Failed to execute #{ command }" unless system( command )
end


def ps_entry_of name
  `ps -ef | grep -w "#{ name } " | grep -v grep`
end


def cucumber_log name
  File.join Trema.log_directory, name
end


def new_tmp_log
  system "rm #{ Trema.log_directory }/tmp.*" # cleanup
  `mktemp --tmpdir=#{ Trema.log_directory }`.chomp  
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
