begin
  unless Dir.glob(File.join(__dir__, '../*.gemspec')).empty?
    require 'bundler/gem_tasks'
  end
rescue LoadError
  task :build do
    $stderr.puts 'Bundler is disabled'
  end
  task :install do
    $stderr.puts 'Bundler is disabled'
  end
  task :release do
    $stderr.puts 'Bundler is disabled'
  end
end
