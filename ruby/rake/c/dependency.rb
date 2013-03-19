require "pstore"
require "trema/path"


module Rake
  module C
    module Dependency
      @@store = {}


      def self.read name, file
        dump_of( name ).transaction( true ) do | store |
          store[ file ]
        end || []
      end


      def self.write name, file, dependency
        dump_of( name ).transaction( false ) do | store |
          store[ file ] = dependency
        end
      end


      def self.dump_of name
        @@store[ name ] ||= PStore.new( path( name ) )
      end


      def self.path name
        File.join Trema.home, ".#{ name }.depends"
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
