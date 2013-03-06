require "rake/c/library-task"


module Rake
  module C
    class StaticLibraryTask < LibraryTask
      ##########################################################################
      private
      ##########################################################################


      def generate_library
        if not uptodate?( target_path, objects )
          sh "ar -cq #{ target_path } #{ objects.join ' ' }"
          sh "ranlib #{ target_path }"
        end
      end


      def target_file_name
        @library_name + ".a"
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
