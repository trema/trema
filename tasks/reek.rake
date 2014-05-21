# encoding: utf-8
begin
  require 'reek/rake/task'

  Reek::Rake::Task.new do |t|
    t.fail_on_error = false
    t.verbose = false
    t.ruby_opts = ['-rubygems']
    t.reek_opts = '--quiet'
    t.source_files = FileList['ruby/**/*.rb', 'src/**/*.rb']
  end
rescue LoadError
  task :reek do
    $stderr.puts 'Reek is disabled'
  end
end
