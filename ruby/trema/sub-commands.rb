#
# Sub commands of trema.
#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
#
# Copyright (C) 2008-2011 NEC Corporation
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


require "irb"
require "optparse"
require "trema/cli"
require "trema/common-commands"
require "trema/dsl"
require "trema/ofctl"
require "trema/util"
require "trema/version"


include Trema::Util


$verbose = false
$run_as_daemon = false


class Trema::SubCommands
  def initialize
    @dsl_parser = Trema::DSL::Parser.new
    @options = OptionParser.new
  end


  def run
    sanity_check

    @options.banner = "Usage: #{ $0 } run [OPTIONS ...]"

    @options.on( "-c", "--conf FILE" ) do | v |
      @config_file = v
    end
    @options.on( "-d", "--daemonize" ) do
      $run_as_daemon = true
    end

    @options.separator ""
    add_help_option
    add_verbose_option

    @options.parse! ARGV

    cleanup_current_session

    if $run_as_daemon
      Trema::DSL::Runner.new( load_config ).daemonize
    else
      begin
        Trema::DSL::Runner.new( load_config ).run
      ensure
        cleanup_current_session
      end
    end
  end


  def start_shell
    require "trema"
    require "trema/shell-commands"
    require "tempfile"
    f = Tempfile.open( "irbrc" )
    f.print <<EOF
ENV[ "TREMA_HOME" ] = Trema.home
@context = Trema::DSL::Context.new
EOF
    f.close
    load f.path
    IRB.start
  end


  def killall
    @options.banner = "Usage: #{ $0 } killall [OPTIONS ...]"

    add_help_option
    add_verbose_option

    @options.parse! ARGV

    cleanup_current_session
  end


  def kill
    @options.banner = "Usage: #{ $0 } kill NAME [OPTIONS ...]"

    add_help_option
    add_verbose_option

    @options.parse! ARGV

    switch = @dsl_parser.load_current.switches[ ARGV[ 0 ] ]
    switch.shutdown!
  end


  def send_packets
    sanity_check

    source = nil
    dest = nil
    cli_options = {}

    @options.banner = "Usage: #{ $0 } send_packets --source HOSTNAME --dest HOSTNAME [OPTIONS ...]"

    @options.on( "-s", "--source HOSTNAME" ) do | v |
      source = @dsl_parser.load_current.hosts[ v ]
      raise "Unknown host: #{ v }" if source.nil?
    end
    @options.on( "--inc_ip_src [NUMBER]" ) do | v |
      if v
        cli_options[ :inc_ip_src ] = v
      else
        cli_options[ :inc_ip_src ] = true
      end
    end
    @options.on( "-d", "--dest HOSTNAME" ) do | v |
      dest = @dsl_parser.load_current.hosts[ v ]
      raise "Unknown host: #{ v }" if dest.nil?
    end
    @options.on( "--inc_ip_dst [NUMBER]" ) do | v |
      if v
        cli_options[ :inc_ip_dst ] = v
      else
        cli_options[ :inc_ip_dst ] = true
      end
    end
    @options.on( "--tp_src NUMBER" ) do | v |
      cli_options[ :tp_src ] = v
    end
    @options.on( "--inc_tp_src [NUMBER]" ) do | v |
      if v
        cli_options[ :inc_tp_src ] = v
      else
        cli_options[ :inc_tp_src ] = true
      end
    end
    @options.on( "--tp_dst NUMBER" ) do | v |
      cli_options[ :tp_dst ] = v
    end
    @options.on( "--inc_tp_dst [NUMBER]" ) do | v |
      if v
        cli_options[ :inc_tp_dst ] = v
      else
        cli_options[ :inc_tp_dst ] = true
      end
    end
    @options.on( "--pps NUMBER" ) do | v |
      cli_options[ :pps ] = v
    end
    @options.on( "--n_pkts NUMBER" ) do | v |
      cli_options[ :n_pkts ] = v
    end
    @options.on( "--duration NUMBER" ) do | v |
      cli_options[ :duration ] = v
    end
    @options.on( "--length NUMBER" ) do | v |
      cli_options[ :length ] = v
    end
    @options.on( "--inc_payload [NUMBER]" ) do | v |
      if v
        cli_options[ :inc_payload ] = v
      else
        cli_options[ :inc_payload ] = true
      end
    end

    @options.separator ""
    add_help_option
    add_verbose_option

    @options.parse! ARGV

    Trema::Cli.new( source ).send_packets( dest, cli_options )
  end


  def show_stats
    sanity_check

    stats = nil

    @options.banner = "Usage: #{ $0 } show_stats HOSTNAME [OPTIONS ...]"

    @options.on( "-t", "--tx" ) do
      stats = :tx
    end
    @options.on( "-r", "--rx" ) do
      stats = :rx
    end

    @options.separator ""
    add_help_option
    add_verbose_option

    @options.parse! ARGV

    host = @dsl_parser.load_current.hosts[ ARGV[ 0 ] ]
    raise "Unknown host: #{ ARGV[ 0 ] }" if host.nil?

    case stats
    when :tx
      Trema::Cli.new( host ).show_tx_stats
    when :rx
      Trema::Cli.new( host ).show_rx_stats
    else
      puts "Sent packets:"
      Trema::Cli.new( host ).show_tx_stats
      puts "Received packets:"
      Trema::Cli.new( host ).show_rx_stats
    end
  end


  def reset_stats
    sanity_check

    @options.banner = "Usage: #{ $0 } reset_stats [OPTIONS ...]"

    add_help_option
    add_verbose_option

    @options.parse! ARGV

    host = @dsl_parser.load_current.hosts[ ARGV[ 0 ] ]
    Trema::Cli.new( host ).reset_stats
  end


  def dump_flows
    sanity_check

    switch = @dsl_parser.load_current.switches[ ARGV[ 0 ] ]

    @options.banner = "Usage: #{ $0 } dump_flows SWITCH [OPTIONS ...]"

    add_help_option
    add_verbose_option

    @options.parse! ARGV

    puts Trema::Ofctl.new.dump_flows( switch )
  end


  def version
    puts "trema version #{ Trema::VERSION }"
  end


  def usage
    command = ARGV.shift

    ARGV.clear << "--help"
    if command.nil?
      puts <<-EOL
usage: #{ $0 } <COMMAND> [OPTIONS ...]

Trema command-line tool
Type '#{ $0 } help <COMMAND>' for help on a specific command.

Available commands:
  run            - runs a trema application.
  kill           - terminates a trema process.
  killall        - terminates all trema processes.
  send_packets   - sends UDP packets to destination host.
  show_stats     - shows stats of packets.
  reset_stats    - resets stats of packets.
  dump_flows     - print all flow entries.
EOL
    elsif method_for( command )
      __send__ method_for( command )
    else
      STDERR.puts "Type '#{ $0 } help' for usage."
      exit false
    end
  end


  ################################################################################
  private
  ################################################################################


  def add_help_option
    @options.on( "-h", "--help" ) do
      puts @options.to_s
      exit 0
    end
  end


  def add_verbose_option
    @options.on( "-v", "--verbose" ) do
      $verbose = true
    end
  end


  def load_config
    config = nil

    if @config_file
      config = @dsl_parser.parse( @config_file )
    elsif FileTest.exists?( "./trema.conf" )
      config = @dsl_parser.parse( "./trema.conf" )
    else
      config = Trema::DSL::Context.new
    end

    if ARGV[ 0 ]
      controller_file = ARGV[ 0 ].split.first
      if c_controller?
        stanza = Trema::DSL::App.new
        stanza.path controller_file
        stanza.options ARGV[ 0 ].split[ 1..-1 ]
        Trema::App.new( stanza )
      else
        # Ruby controller
        require "trema"
        ARGV.replace ARGV[ 0 ].split
        $LOAD_PATH << File.dirname( controller_file )
        Trema.module_eval IO.read( controller_file )
      end
    end

    config
  end


  def c_controller?
    /ELF/=~ `file #{ ARGV[ 0 ].split.first }`
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
