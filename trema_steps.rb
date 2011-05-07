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


Given /^I terminated all trema services$/ do
  run "sudo ./trema kill"
end


Given /^I terminate all trema services$/ do
  Given "I terminated all trema services"
end


When /^\*\*\* sleep (\d+) \*\*\*$/ do | sec |
  sleep sec.to_i
end


When /^wait until "([^"]*)" is up$/ do | process |
  loop do
    break if FileTest.exists?( File.join( Trema.tmp, "#{ process }.pid" ) )
    sleep 1
  end
end


When /^wait until trema session is closed$/ do
  loop do
    sleep 1    
    break unless FileTest.exists?( File.join( Trema.tmp, ".context" ) )
  end
end


Then /^([^\s]*) is terminated$/ do | name |
  ps_entry_of( name ).should be_empty
end


Then /^([^\s]*) is started$/ do | name |
  ps_entry_of( name ).should_not be_empty
end


Then /^switch_manager should be killed$/ do
  IO.read( @trema_log ).should match( /^Terminating switch_manager/ )
end


Then /^the total number of tx packets should be:$/ do | table |
  table.hashes[ 0 ].each_pair do | host, n |
    stats = `./trema show_stats #{ host } --tx`
    next if stats.split.size <= 1
    `./trema show_stats #{ host } --tx`.split[ 1..-1 ].inject( 0 ) do | sum, each |
      # ip_dst,tp_dst,ip_src,tp_src,n_pkts,n_octets
      sum += each.split( "," )[ 4 ].to_i
    end.should == n.to_i
  end
end


Then /^the total number of rx packets should be:$/ do | table |
  table.hashes[ 0 ].each_pair do | host, n |
    stats = `./trema show_stats #{ host } --rx`
    next if stats.split.size <= 1
    stats.split[ 1..-1 ].inject( 0 ) do | sum, each |
      if each.split( "," )[ 0 ].split( "." ).last == host.split( // ).last
        sum += each.split( "," )[ 4 ].to_i
      end
      sum
    end.should == n.to_i
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
