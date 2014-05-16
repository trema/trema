begin
  require 'rubocop/rake_task'
  Rubocop::RakeTask.new do |task|
    task.patterns =
      [
        'Gemfile',
        'Rakefile',
        'bin/*',
        'cruise.rb',
        'features/**/*.rb',
        'ruby/**/*.rb',
        'spec/**/*.rb',
        'src/**/*.rb',
        'tasks/*.rake',
        'trema.gemspec'
      ]
  end
rescue LoadError
  task :rubocop do
    $stderr.puts 'Rubocop is disabled'
  end
end
