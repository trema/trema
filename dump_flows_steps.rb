Then(/^the switch "(.*?)" has (\d+) flow entries$/) do |switch, num_entries|
  command = "trema dump_flows #{switch} -S."
  step "I run `#{command}`"
  expect(output_from(command).split("\n").size - 1).to eq(num_entries.to_i)
end
