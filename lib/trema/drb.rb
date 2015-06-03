require 'drb'
require 'phut'

# OpenFlow controller programming framework.
module Trema
  def self.trema_process(controller_name, socket_dir)
    Phut.socket_dir = socket_dir
    socket_path = File.join(Phut.socket_dir, "trema.#{controller_name}.ctl")
    unless FileTest.socket?(socket_path)
      fail %(Controller process "#{controller_name}" does not exist.)
    end
    DRbObject.new_with_uri('drbunix:' + socket_path)
  end

  def self.trema_processes(socket_dir = Phut.socket_dir)
    Phut.socket_dir = socket_dir
    Dir.glob(File.join Phut.socket_dir, 'trema.*.ctl').map do |each|
      DRbObject.new_with_uri('drbunix:' + each)
    end
  end

  def self.fetch(name, socket_dir)
    trema_processes(socket_dir).each do |trema|
      begin
        return trema.fetch(name)
      rescue
        next
      end
    end
    fail %("#{name}" does not exist.)
  end
end
