Before do
  @aruba_timeout_seconds = 5
end

Before('@sudo') do
  fail 'sudo authentication failed' unless system 'sudo -v'
  @aruba_timeout_seconds = 10
end

After do
  run 'trema killall'
  sleep 5
end
