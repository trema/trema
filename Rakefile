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


require "rubygems"

require "rake/tasklib"
require "cucumber/rake/task"
require "flay"
require "flay_task"
require "flog"
require "reek/rake/task"
require "roodi"
require "roodi_task"
require "rspec/core/rake_task"


desc "Generate a monolithic rant file"
task "build.rb" do
  sh "rant-import --force --auto .mono.rant"
end


desc "Run all examples with RCov"
RSpec::Core::RakeTask.new do | t |
  t.pattern = [ "spec/**/*_spec.rb", "src/examples/**/*_spec.rb" ]
  t.rspec_opts = "--color --format documentation --profile"
end


desc "Enforce Ruby code quality with static analysis of code"
task :quality => [ :reek, :roodi, :flog, :flay ]


#
# See the follwing URL for details:
# http://wiki.github.com/kevinrutherford/reek/rake-task
#
Reek::Rake::Task.new do | t |
  t.fail_on_error = true
  t.verbose = false
  t.ruby_opts = [ "-rubygems" ]
  t.reek_opts = "--quiet"
  t.source_files = "ruby/**/*.rb"
end


RoodiTask.new do | t |
  t.patterns = %w(ruby/**/*.rb spec/**/*.rb features/**/*.rb)
end


desc "Analyze for code complexity"
task :flog do
  flog = Flog.new( :continue => true )
  flog.flog [ "ruby" ]
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


FlayTask.new do | t |
  # add directories such as app, bin, spec and test if need be.
  t.dirs = %w( ruby )
  t.threshold = 0
end


Cucumber::Rake::Task.new do | t |
  t.cucumber_opts = [ "features" ]
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
