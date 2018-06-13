# frozen_string_literal: true

begin
  unless Dir.glob(File.join(__dir__, '../*.gemspec')).empty?
    require 'bundler/gem_tasks'
  end
rescue LoadError
  task :build do
    warn 'Bundler is disabled'
  end
  task :install do
    warn 'Bundler is disabled'
  end
  task :release do
    warn 'Bundler is disabled'
  end
end
