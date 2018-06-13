RELISH_PROJECT = 'trema/trema'

task default: [:test, :quality]
# task test: [:spec, :cucumber]
task test: [:spec]
task quality: [:rubocop, :reek]
task travis: [:quality]

Dir.glob('tasks/*.rake').each { |each| import each }
