Given(/^I use OpenFlow 1\.0$/) do
  @open_flow_version = :open_flow10
end

Given(/^I use OpenFlow 1\.3$/) do
  @open_flow_version = :open_flow13
end

# rubocop:disable LineLength
When(/^I trema run "([^"]*)"( interactively)? with the configuration "([^"]*)"$/) do |controller_file, interactive, configuration_file|
  open_flow_option = @open_flow_version == :open_flow13 ? ' --openflow13' : ''
  run_arguments = "#{File.join '..', '..', controller_file}#{open_flow_option} -c #{configuration_file}"
  if interactive
    step %(I run `trema run #{run_arguments}` interactively)
  else
    step %(I successfully run `trema run #{run_arguments} -d`)
  end
  step %(I successfully run `sleep 3`)
end
# rubocop:enable LineLength

When(/^I trema killall "([^"]*)"$/) do |controller_name|
  step %(I successfully run `trema killall #{controller_name}`)
end

# rubocop:disable LineLength
Then(/^the log file "([^"]*)" should contain following messages:$/) do |log_file, messages|
  step %(a file named "#{log_file}" should exist)
  messages.rows.flatten.each do |each|
    step %(the file "#{log_file}" should contain "#{each}")
  end
end
# rubocop:enable LineLength
