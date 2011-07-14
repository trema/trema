module Trema
	class Stats

		def initialize fields, options
			fields.each do |field|
				instance_variable_set("@#{field}", options[field.intern])
			end
		end

		def to_s
			str=""
			instance_variables.sort.each do |var|
				str += "#{var[1..var.length]}: #{instance_variable_get(var).to_s}\n"
			end
			str
		end
	end
end
