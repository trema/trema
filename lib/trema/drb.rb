require 'drb'
require 'phut'

# OpenFlow controller programming framework.
module Trema
  def self.socket_dir
    Phut.socket_dir
  end

  def self.trema_process(socket_dir = Phut.socket_dir, check = false)
    Phut.socket_dir = socket_dir
    path = File.expand_path(File.join Phut.socket_dir, 'trema.ctl')
    if check && !FileTest.socket?(path)
      fail "Socket file #{path} does not exist."
    end
    DRbObject.new_with_uri('drbunix:' + path)
  end

  def self.controller_process(socket_dir)
    trema_process(socket_dir).controller
  end
end
