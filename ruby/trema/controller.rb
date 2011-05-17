module Trema
  class Controller
    @@list = {}


    def self.name
      to_s.split( "::" ).last      
    end
    

    def self.each &block
      @@list.values.each do | each |
        block.call each
      end
    end
    

    def self.inherited subclass
      controller = subclass.new
      @@list[ controller.name ] = controller
    end
    

    def self.[] name
      @@list[ name ]
    end


    def name
      self.class.name
    end
  end
end
