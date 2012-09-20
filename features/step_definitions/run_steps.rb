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


When /^I try to run "([^"]*)"$/ do | command |
  @log ||= new_tmp_log
  run "#{ command } >> #{ @log }"
end


When /^I try to run "([^"]*)" \(log = "([^"]*)"\)$/ do | command, log_name |
  run "#{ command } > #{ cucumber_log log_name } 2>&1"
end


When /^I try trema run "([^"]*)" with following configuration \((.*)\):$/ do | args, options, config |
  verbose = if /verbose/=~ options
              "--verbose"
            else
              ""
            end
  @log ||= new_tmp_log

  trema_run = Proc.new do
    Tempfile.open( "trema.conf" ) do | f |
      f.puts config
      f.flush
      run "./trema #{ verbose } run \"#{ args }\" -c #{ f.path } >> #{ @log } 2>&1"
    end
  end

  if /background/=~ options
    pid = Process.fork do
      trema_run.call
    end
    Process.detach pid
  else
    trema_run.call
  end
end


Given /^I try trema run "([^"]*)" example with following configuration \(backgrounded\):$/ do | example, config |
  controller = nil
  name = nil
  if /\.rb\Z/=~ example
    controller = "./src/examples/#{ File.basename( example, ".rb" ).tr( "-", "_" ) }/#{ example }"
    name = File.basename( example, ".rb" ).camelize
  else
    controller = "./objects/examples/#{ example }/#{ example }"
    name = example
  end
  step %{I try trema run "#{ controller }" with following configuration (backgrounded):}, config
  step %{wait until "#{ name }" is up}
end


When /^I try trema run "([^"]*)" with following configuration:$/ do | args, config |
  step "I try trema run \"#{ args }\" with following configuration (no options):", config
end


Then /^"([^"]*)" exits abnormally with an error message:$/ do | command, message |
  log = "error.log"
  step %{I try to run "#{ command }" (log = "#{ log }")} rescue error_occured = true
  error_occured.should be_true
  step %{the content of "#{ log }" should be:}, message
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
