# encoding: utf-8
task :travis => [:build_trema, 'spec:travis', :rubocop]
