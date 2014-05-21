task :travis => [:build_trema, 'spec:travis', :rubocop]
