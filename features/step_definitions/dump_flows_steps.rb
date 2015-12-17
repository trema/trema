Then(/^the switch "(.*?)" has (\d+) flow entr(?:y|ies)$/) do |switch, number|
  # FIXME: Read TREMA_SOCKET_DIR in trema dump_flows
  command = "trema dump_flows #{switch} -S ."
  step "I successfully run `#{command}`"
  dump_flows = aruba.command_monitor.find(Aruba.platform.detect_ruby(command))
  expect(dump_flows.output.split("\n").size - 1).to eq(number.to_i)
end

Then(/^the switch "(.*?)" has no flow entry$/) do |switch|
  step %(the switch "#{switch}" has 0 flow entry)
end
