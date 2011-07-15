require "trema/stats"
module Trema
	class TableStatsReply < Stats
		FIELDS = %w(table_id name wildcards max_entries ) +
			%w(active_count lookup_count matched_count)

		FIELDS.each { |field| attr_reader field.intern }

		NAME = self.name

    def initialize options
			super FIELDS, options
		end

		def to_s
			str="#{NAME}\n" + super.to_s
		end
	end
end
