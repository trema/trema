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


Then /^the output should be:$/ do | string |
  IO.read( @log ).chomp.should == string.chomp
end


Then /^the output of trema should be:$/ do | string |
  IO.read( @trema_log ).chomp.should == string.chomp
end


Then /^the output of trema should include:$/ do | string |
  string.chomp.split( "\n" ).each do | each |
    IO.read( @trema_log ).split( "\n" ).should include( each )
  end
end


Then /^"([^"]*)" should be executed with option = "([^"]*)"$/ do | executable, options |
  IO.read( @trema_log ).should match( Regexp.new "#{ executable } #{ options }" )
end


Then /^the log file "([^"]*)" should be:$/ do | log_name, string |
  log = File.join( Trema.log_directory, log_name )
  IO.read( log ).chomp.should == string.chomp
end


Then /^the log file "([^"]*)" should include:$/ do | log_name, string |
  log = File.join( Trema.log_directory, log_name )
  IO.read( log ).split( "\n" ).should include( string )
end


Then /^the log file "([^"]*)" should include "([^"]*)" x (\d+)$/ do | log, message, n |
  IO.read( log ).split( "\n" ).inject( 0 ) do | matched, each |
    matched += 1 if each.include?( message )
    matched
  end.should == n.to_i
end


Then /^the content of "([^"]*)" and "([^"]*)" should be identical$/ do | log1, log2 |
  IO.read( log1 ).size.should > 0
  IO.read( log2 ).size.should > 0
  IO.read( log1 ).chomp.should == IO.read( log2 ).chomp
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
