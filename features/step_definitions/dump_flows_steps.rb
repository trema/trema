# rubocop:disable LineLength

Then(/^the switch "(.*?)" has (\d+) flow entr(?:y|ies)$/) do |switch, num_entries|
  command = "trema dump_flows #{switch} -S."
  step "I run `#{command}`"
  expect(output_from(command).split("\n").size - 1).to eq(num_entries.to_i)
end

Then(/^the switch "(.*?)" has no flow entry$/) do |switch|
  step %(the switch "#{switch}" has 0 flow entry)
end

# rubocop:enable LineLength
