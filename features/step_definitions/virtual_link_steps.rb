# rubocop:disable LineLength

Then(/^virtual links should not exist$/) do
  step %(I run `bash -c 'ifconfig | grep "^link[0-9]\+-[0-9]\+" > virtual_links.txt'`)
  step 'the file "virtual_links.txt" should not contain "link"'
end

# rubocop:enable LineLength
