# encoding: utf-8
begin
  require 'flay'
  require 'flay_task'

  FlayTask.new do | t |
    t.dirs = FileList['ruby/**/*.rb', 'src/**/*.rb'].map do |each|
      each[/[^\/]+/]
    end.uniq
    t.threshold = 0
    t.verbose = true
  end
rescue LoadError
  task :flay do
    $stderr.puts 'Flay is disabled'
  end
end
