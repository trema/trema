#
# Spawns a process and connects pipes to its stdout/stderr and obtain
# its exit code.
#
#
# == Example:
#
#  SubProcess.create do | shell |
#    # Add some hooks here
#    shell.on_stdout do | line |
#      log line
#      $stdout.puts line
#    end
#    shell.on_stderr do | line |
#      log line
#      $stderr.puts line
#    end
#    shell.on_failure do
#      raise "'#{ command }' failed."
#    end
#
#    # Spawn a subprocess
#    shell.exec command
#  end
#
#
# == Hooks:
#
# <tt>on_stdout</tt>:: Executed when a new line arrived from sub-process's stdout.
# <tt>on_stderr</tt>:: Executed when a new line arrived from sub-process's stderr.
# <tt>on_exit</tt>:: Executed when sub process exited.
# <tt>on_success</tt>:: Executed when sub process exited successfully.
# <tt>on_failure</tt>:: Executed when sub process exited with an error.
#
#
# == Credit:
#
# Copyright (C) 2008-2013 NEC Corporation
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


module SubProcess
  class Command
    attr_reader :command
    attr_reader :env


    def initialize command, env = {}
      @command = command
      @env = env
    end


    def start
      @env.each_pair do | key, value |
        ENV[ key ]= value
      end
      Kernel.exec @command
    end
  end


  class PipeSet
    attr_reader :stdin
    attr_reader :stdout
    attr_reader :stderr


    def initialize stdin, stdout, stderr
      @stdin = stdin
      @stdout = stdout
      @stderr = stderr
    end


    def close
      [ @stdin, @stdout, @stderr ].each do | each |
        unless each.closed?
          each.close
        end
      end
    end
  end


  class Process
    def initialize
      stdin, stdout, stderr = Array.new( 3 ) { IO.pipe }
      @child = SubProcess::PipeSet.new( stdin[ 1 ], stdout[ 0 ], stderr[ 0 ] )
      @parent = SubProcess::PipeSet.new( stdin[ 0 ], stdout[ 1 ], stderr[ 1 ] )
    end


    def wait
      ::Process.wait @pid
    end


    def popen command, &block
      @pid = fork_child( command )
      # Parent process
      @parent.close
      begin
        yield @child.stdout, @child.stderr
      ensure
        @child.close
      end
      self
    end


    ############################################################################
    private
    ############################################################################


    def fork_child command
      Kernel.fork do
        @child.close
        redirect_child_io
        command.start
      end
    end


    def redirect_child_io
      STDIN.reopen @parent.stdin
      STDOUT.reopen @parent.stdout
      STDERR.reopen @parent.stderr
      @parent.close
    end
  end


  class IoHandlerThread
    def initialize io, method
      @io = io
      @method = method
    end


    def start
      Thread.new( @io, @method ) do | io, method |
        while io.gets do
          method.call $LAST_READ_LINE
        end
      end
    end
  end


  class Shell
    def self.open debug_options = {}, &block
      block.call self.new( debug_options )
    end


    def initialize debug_options
      @debug_options = debug_options
    end


    def child_status
      $CHILD_STATUS
    end


    def on_stdout &block
      @on_stdout = block
    end


    def on_stderr &block
      @on_stderr = block
    end


    def on_exit &block
      @on_exit = block
    end


    def on_success &block
      @on_success = block
    end


    def on_failure &block
      @on_failure = block
    end


    def exec command, env = { "LC_ALL" => "C" }
      on_failure { raise "command #{ command } failed" } unless @on_failure
      SubProcess::Process.new.popen SubProcess::Command.new( command, env ) do | stdout, stderr |
        handle_child_output stdout, stderr
      end.wait
      handle_exitstatus
      self
    end


    ############################################################################
    private
    ############################################################################


    def handle_child_output stdout, stderr
      tout = SubProcess::IoHandlerThread.new( stdout, method( :do_stdout ) ).start
      terr = SubProcess::IoHandlerThread.new( stderr, method( :do_stderr ) ).start
      tout.join
      terr.join
    end


    # run hooks ################################################################


    def handle_exitstatus
      do_exit
      if child_status.exitstatus == 0
        do_success
      else
        do_failure
      end
    end


    def do_stdout line
      if @on_stdout
        @on_stdout.call line
      end
    end


    def do_stderr line
      if @on_stderr
        @on_stderr.call line
      end
    end


    def do_failure
      if @on_failure
        @on_failure.call
      end
    end


    def do_success
      if @on_success
        @on_success.call
      end
    end


    def do_exit
      if @on_exit
        @on_exit.call
      end
    end
  end


  def create debug_options = {}, &block
    Shell.open debug_options, &block
  end
  module_function :create
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
