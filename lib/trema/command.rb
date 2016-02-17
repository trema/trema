require 'English'

module Trema
  # trema command
  # rubocop:disable ClassLength
  class Command
    def self.unix_domain_socket(name, check = false)
      path = File.expand_path(File.join(Phut.socket_dir, "#{name}.ctl"))
      if check && !FileTest.socket?(path)
        raise "Socket file #{path} does not exist."
      end
      'drbunix:' + path
    end

    attr_reader :controller

    # rubocop:disable AbcSize
    # rubocop:disable MethodLength
    def run(args, options)
      @args = args
      @daemon = options[:daemonize]
      $LOAD_PATH.unshift File.expand_path(File.dirname(@args.first))
      load @args.first
      port_number = (options[:port] || Controller::DEFAULT_TCP_PORT).to_i
      @controller =
        Controller.create(port_number, options.fetch(:logging_level))

      trap_signals
      create_pid_file
      start_phut(options[:conf])

      if options[:daemonize]
        run_as_daemon { start_controller_and_drb_threads }
      else
        start_controller_and_drb_threads
      end
    end
    # rubocop:enable MethodLength
    # rubocop:enable AbcSize

    def kill(name)
      @phut.fetch(name).stop
    end

    def delete_link(endpoint1, endpoint2)
      target = @phut.fetch([endpoint1, endpoint2])
      begin
        target.stop
      rescue
        true
      end
    end

    # rubocop:disable CyclomaticComplexity
    def killall
      @controller.logger.debug 'Shutting down...' if @controller
      @controller.stop
      @controller_thread.kill if @controller_thread
      @phut_run_thread.kill if @phut_run_thread
      @phut.stop if @phut
      FileUtils.rm pid_file if FileTest.exists?(pid_file)
      DRb.stop_service
      exit 0 if @daemon
    end
    # rubocop:enable CyclomaticComplexity

    def up(name)
      @phut.fetch(name).run
    end

    def port_up(switch_name, port)
      switch = @phut.fetch(switch_name)
      switch.bring_port_up(port)
    end

    def port_down(switch_name, port)
      switch = @phut.fetch(switch_name)
      switch.bring_port_down(port)
    end

    def fetch(name)
      @phut.fetch(name)
    rescue KeyError
      raise "Host not found: #{name}"
    end

    private

    # rubocop:disable MethodLength
    def start_phut(config_file)
      return unless config_file
      system 'sudo -v'
      @phut = Phut::Parser.new(@controller.logger).parse(config_file)
      @phut_run_thread = Thread.start { @phut.run }
      @phut_run_thread.join
      Thread.start { start_sudo_credential_update }
    rescue ScriptError, NameError
      killall
      raise $ERROR_INFO
    rescue StandardError
      raise $ERROR_INFO unless @stop
    end
    # rubocop:enable MethodLength

    def start_controller_and_drb_threads
      DRb.start_service Command.unix_domain_socket(@controller.name), self
      @controller_thread = Thread.new { @controller.run @args[1..-1] }
      @controller_thread.abort_on_exception = true
      DRb.thread.join
    rescue
      killall
      raise $ERROR_INFO
    end

    def run_as_daemon
      fork do
        redirect_stdio_to_devnull
        update_pid_file
        yield
      end
    end

    def redirect_stdio_to_devnull
      open('/dev/null', 'r+') do |devnull|
        $stdin.reopen devnull
        $stdout.reopen devnull
        $stderr.reopen devnull
      end
    end

    def stop
      @stop = true
    end

    # rubocop:disable MethodLength
    def trap_signals
      @killall_thread = Thread.start do
        loop do
          if @stop
            killall
            break
          end
          sleep 1
        end
      end
      @killall_thread.abort_on_exception = true
      Signal.trap(:TERM) { stop }
      Signal.trap(:INT) { stop }
    end
    # rubocop:enable MethodLength

    def create_pid_file
      raise "#{name} is already running." if running?
      update_pid_file
    end

    def update_pid_file
      File.open(pid_file, 'w') { |file| file << Process.pid }
    end

    def pid_file
      File.join Phut.pid_dir, "#{name}.pid"
    end

    def running?
      FileTest.exists? pid_file
    end

    def name
      @controller.name
    end

    def start_sudo_credential_update
      loop do
        system 'sudo -v'
        sleep 60
      end
    end
  end
  # rubocop:enable ClassLength
end
