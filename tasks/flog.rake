begin
  require 'flog'

  desc 'Analyze for code complexity'
  task :flog do
    flog = Flog.new(:continue => true)
    flog.flog(*FileList['ruby/**/*.rb', 'src/**/*.rb'])
    threshold = 10

    bad_methods = flog.totals.select do |name, score|
      !(/##{flog.no_method}$/ =~ name) && score > threshold
    end
    bad_methods.sort do |a, b|
      a[1] <=> b[1]
    end.reverse.each do |name, score|
      puts '%8.1f: %s' % [score, name]
    end
    unless bad_methods.empty?
      $stderr.puts "#{bad_methods.size} methods have a flog complexity > #{threshold}"
    end
  end
rescue LoadError
  task :flog do
    $stderr.puts 'Flog is disabled'
  end
end
