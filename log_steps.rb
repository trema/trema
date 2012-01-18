#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
#
# Copyright (C) 2008-2012 NEC Corporation
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


Then /^I should not get errors$/ do
  step "the output should be:", ""
end


Then /^the output should include:$/ do | string |
  string.chomp.split( "\n" ).each do | each |
    IO.read( @log ).split( "\n" ).should include( each )
  end
end


Then /^"([^"]*)" should be executed with option = "([^"]*)"$/ do | executable, options |
  IO.read( @log ).should match( Regexp.new "#{ executable } #{ options }" )
end


Then /^the log file "([^"]*)" should be:$/ do | log_name, string |
  IO.read( cucumber_log log_name ).chomp.should == string.chomp
end


Then /^the log file "([^"]*)" should include:$/ do | log_name, string |
  IO.read( cucumber_log log_name ).split( "\n" ).should include( string )
end


Then /^the log file "([^"]*)" should match:$/ do | log_name, string |
  IO.read( cucumber_log log_name ).should match( Regexp.new string )
end


Then /^the log file "([^"]*)" should include "([^"]*)" x (\d+)$/ do | log, message, n |
  IO.read( cucumber_log log ).split( "\n" ).inject( 0 ) do | matched, each |
    matched += 1 if each.include?( message )
    matched
  end.should == n.to_i
end


Then /^the content of "([^"]*)" should be:$/ do | log_name, string |
  IO.read( cucumber_log log_name ).chomp( "" ).should == string.chomp
end


Then /^the content of "([^"]*)" and "([^"]*)" should be identical$/ do | log1, log2 |
  IO.read( cucumber_log log1 ).size.should > 0
  IO.read( cucumber_log log2 ).size.should > 0
  IO.read( cucumber_log log1 ).chomp.should == IO.read( cucumber_log log2 ).chomp
end


Then /^"([^"]*)" should contain some flow entries$/ do | log |
  IO.read( cucumber_log log ).size.should > 0
  IO.read( cucumber_log log ).split( "\n" )[ 2..-1 ].each do | each |
    each.should match( /actions=FLOOD/ )
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
