Before do
  @aruba_timeout_seconds = 5
end

Before('@sudo') do
  ENV['TREMA_LOG_DIR'] = '.'
  ENV['TREMA_PID_DIR'] = '.'
  ENV['TREMA_SOCKET_DIR'] = '.'
  fail 'sudo authentication failed' unless system 'sudo -v'
  @aruba_timeout_seconds = 10
end

After do
  run "trema killall --all -S #{ENV['TREMA_SOCKET_DIR']}"
  sleep 5
end
