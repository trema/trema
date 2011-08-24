module Trema
  class StatsHelper

    
    # Called by each StatsReply subclass to set their instance attributes to a value.
    # 
    # @overload initialize(fields, options={})
    # 
    # @param [Array] fields
    #   an array of attribute names to set.
    # 
    # @param [Hash] options
    #   key/value pairs of attributes.
    #   
    # @return [void]
    #
    def initialize fields, options
      fields.each do |field|
        instance_variable_set( "@#{field}", options[field.intern] )
      end
    end

    
    #
    # @return [String] 
    #   an alphabetically sorted string of attribute name/value pairs.
    #
    def to_s
      str = super.to_s + "\n"
      instance_variables.sort.each do |var|
        str += "#{var[1..var.length]}: #{instance_variable_get( var ).to_s}\n"
      end
      str
    end
  end
end
