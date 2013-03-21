require "popen4"
require "rake/c/dependency"
require "rake/clean"
require "rake/tasklib"


module Rake
  module C
    class LibraryTask < TaskLib
      attr_accessor :cflags
      attr_accessor :includes
      attr_accessor :library_name
      attr_accessor :name
      attr_accessor :target_directory
      attr_writer :sources


      def initialize name, &block
        init name
        block.call self
        define
      end


      def sources
        FileList.new @sources
      end


      ##########################################################################
      private
      ##########################################################################


      def init name
        @name = name
        @includes = []
      end


      def define
        CLEAN.include objects
        CLOBBER.include target_directory
        CLOBBER.include Dependency.path( @library_name )

        task name => [ target_directory, target_path ]
        directory target_directory

        sources.zip( objects ) do | source, object |
          task object => source do | task |
            compile task.name, task.prerequisites[ 0 ]
          end
        end

        file target_path => objects do | task |
          generate_library
        end
      end


      def generate_library
        raise NotImplementedError, "Override this!"
      end


      def target_file_name
        raise NotImplementedError, "Override this!"
      end


      def target_path
        File.join @target_directory, target_file_name
      end


      def objects
        sources.collect do | each |
          File.join @target_directory, File.basename( each ).ext( ".o" )
        end
      end


      def compile o_file, c_file
        return if uptodate?( o_file, [ c_file ] + Dependency.read( library_name, o_file ) )
        autodepends = run_gcc_H( "gcc -H #{ gcc_cflags } -fPIC #{ gcc_I_options } -c #{ c_file } -o #{ o_file }" )
        Dependency.write( library_name, o_file, autodepends )
      end


      def run_gcc_H command
        autodepends = []

        puts command
        status = POpen4.popen4( command ) do | stdout, stderr, stdin, pid |
          stdin.close
          stderr.each do | line |
            case line
              when /^\./
                autodepends << line.sub( /^\.+\s+/, "" ).strip
              when /Multiple include guards/
                # Filter out include guards warnings.
                stderr.each do | line |
                  if line =~ /:$/
                    puts line
                    break
                  end
                end
              else
                puts line
            end
          end
        end
        fail "gcc failed" if status.exitstatus != 0

        autodepends
      end


      def gcc_cflags
        @cflags.join " "
      end


      def gcc_I_options
        @includes.collect do | each |
          "-I#{ each }"
        end.join( " " )
      end
    end
  end
end


require "rake/c/shared-library-task"
require "rake/c/static-library-task"


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
