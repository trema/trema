#
# trema usage (help) command.
#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
#
# Copyright (C) 2008-2012 NEC Corporation
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2, as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#


module Trema
  module Command
    def usage
      command = ARGV.shift

      ARGV.clear << "--help"
      if command.nil?
        puts <<-EOL
usage: trema <COMMAND> [OPTIONS ...]

Trema command-line tool
Type 'trema help <COMMAND>' for help on a specific command.

Available commands:
  run            - runs a trema application.
  kill           - terminates a trema process.
  up             - starts a killed trema process again.
  killall        - terminates all trema processes.
  send_packets   - sends UDP packets to destination host.
  show_stats     - shows stats of packets.
  reset_stats    - resets stats of packets.
  dump_flows     - print all flow entries.
  ruby           - opens in your browser Trema's Ruby API documentation.
  version        - displays the current runtime version.
EOL
      elsif method_for( command )
        __send__ method_for( command )
      else
        STDERR.puts "Type 'trema help' for usage."
        exit false
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
