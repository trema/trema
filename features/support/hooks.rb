Before do
  @aruba_timeout_seconds = 5
end

Before('@sudo') do
  fail 'sudo authentication failed' unless system 'sudo -v'
  @aruba_timeout_seconds = 15
end

After do
  run 'trema killall --all -S .'
  sleep 10
end
