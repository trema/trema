# encoding: utf-8

def cbench_command
  File.join Trema.objects, 'oflops/bin/cbench'
end

def cbench_latency_mode_options
  '--port 6653 --switches 1 --loops 10 --delay 1000'
end

def cbench_throughput_mode_options
  cbench_latency_mode_options + ' --throughput'
end

def cbench(controller, options)
  sh "#{controller}"
  sh "#{cbench_command} #{options}"
ensure
  sh './trema killall'
end

def cbench_c_controller
  './trema run ./objects/examples/cbench_switch/cbench_switch -d'
end

def cbench_ruby_controller
  './trema run src/examples/cbench_switch/cbench-switch.rb -d'
end

def run_cbench(controller)
  cbench controller, cbench_latency_mode_options
  cbench controller, cbench_throughput_mode_options
end

def cbench_profile(options)
  valgrind = 'valgrind --tool=callgrind --trace-children=yes'
  begin
    sh "#{valgrind} #{cbench_c_controller}"
    sh "#{cbench_command} #{options}"
  ensure
    sh './trema killall'
  end
end

CLEAN.include FileList['callgrind.out.*']

desc 'Run the cbench switch controller to benchmark'
task 'cbench' => 'cbench:ruby'

desc 'Run the c cbench switch controller to benchmark'
task 'cbench:c' => :default do
  run_cbench cbench_c_controller
end

desc 'Run the ruby cbench switch controller to benchmark'
task 'cbench:ruby' => :default do
  run_cbench cbench_ruby_controller
end

desc 'Run cbench with profiling enabled.'
task 'cbench:profile' => :default do
  cbench_profile cbench_latency_mode_options
  cbench_profile cbench_throughput_mode_options
end
