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


require "rubygems"
require "bundler"


begin
  Bundler.setup :default, :development
rescue Bundler::BundlerError => e
  $stderr.puts e.message
  $stderr.puts "Run `bundle install` to install missing gems"
  exit e.status_code
end


require "rake"

task :default do
  system "./build.rb"
end


require "jeweler"

Jeweler::Tasks.new do |gem|
  # gem is a Gem::Specification... see http://docs.rubygems.org/read/chapter/20 for more options
  gem.name = "trema"
  gem.homepage = "http://github.com/trema/trema"
  gem.license = "GPL2"
  gem.summary = %Q{Full-Stack OpenFlow Framework for Ruby/C}
  gem.description = %Q{Trema is a full-stack, easy-to-use framework for developing OpenFlow controllers in Ruby/C}
  gem.email = "yasuhito@gmail.com"
  gem.authors = ["Yasuhito Takamiya"]
  gem.bindir = ["."]
  gem.executables = ["trema","trema-config"]
  gem.extensions = ["Rakefile"]
  # dependencies defined in Gemfile
end
Jeweler::RubygemsDotOrgTasks.new


require "rspec/core"
require "rspec/core/rake_task"

RSpec::Core::RakeTask.new( :spec ) do | spec |
  spec.pattern = FileList[ "spec/**/*_spec.rb" ]
end

RSpec::Core::RakeTask.new( :rcov ) do | spec |
  spec.pattern = "spec/**/*_spec.rb"
  spec.rcov = true
end


require "cucumber/rake/task"
Cucumber::Rake::Task.new( :features )


desc "Enforce Ruby code quality with static analysis of code"
task :quality => [ :reek, :roodi, :flog, :flay ]


require "reek/rake/task"

Reek::Rake::Task.new do | t |
  t.fail_on_error = true
  t.verbose = false
  t.ruby_opts = [ "-rubygems" ]
  t.reek_opts = "--quiet"
  t.source_files = "ruby/**/*.rb"
end


require "roodi"
require "roodi_task"

RoodiTask.new do | t |
  t.verbose = false
  t.patterns = %w(ruby/**/*.rb spec/**/*.rb features/**/*.rb)
end


require "flog"

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


require "flay"
require "flay_task"

FlayTask.new do | t |
  # add directories such as app, bin, spec and test if need be.
  t.dirs = %w( ruby )
  t.threshold = 0
end


desc "Generate a monolithic rant file"
task "build.rb" do
  sh "rant-import --force --auto .mono.rant"
end


require "yard"

YARD::Rake::YardocTask.new do | t |
  t.options << "--debug" << "--verbose" if $trace
end
