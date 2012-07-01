#
# Simple ActiveRecored based application
#
# Author: kazuhiro MIYASHITA <miyakz1192@gmail.com>
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

# build following association 
#   +---------------+ 1    N +------------+
#   | OpenFlowSwitch| <----> |OpenFlowPort|
#   +---------------+        +------------+
#

# ActiveRecord data model as OpenFlowSwitch
class OpenFlowSwitch < ActiveRecord::Base
  has_many :open_flow_ports, :dependent => :destroy
end

# ActiveRecord data model as OpenFlowPort
class OpenFlowPort < ActiveRecord::Base
  belongs_to :open_flow_switch
end

# create or delete table for open_flow_switches
class CreateOpenFlowSwitch < ActiveRecord::Migration

  TABLE = :open_flow_switches

  #create table
  def up
    return if OpenFlowSwitch.table_exists?
    create_table TABLE do |t|
      t.integer :dpid
    end
  end

  #delete table
  def down
    return unless OpenFlowSwitch.table_exists?
    drop_table TABLE
  end
end

class CreateOpenFlowPort < ActiveRecord::Migration

  TABLE = :open_flow_ports

  #create table
  def up
    return if OpenFlowPort.table_exists?
    create_table TABLE do |t|
      t.integer :open_flow_switch_id #for associating to OpenFlowSwitch
      t.integer :no                  #port number
    end
  end

  #delete table
  def down
    return unless OpenFlowPort.table_exists?
    drop_table TABLE
  end
end


class DBConnector
  def self.start
    ActiveRecord::Base.establish_connection(
                                            :adapter  => "sqlite3",
                                            :database => "db.sqlite3",
                                            :timeout  => 5000
                                            ) 
  end
end

class MyController < Controller
  add_timer_event :list_switches, 10, :periodic

  def start
    DBConnector.start
    CreateOpenFlowSwitch.new.up
    CreateOpenFlowPort.new.up
  end
  
  def switch_ready dpid
    send_message dpid, FeaturesRequest.new
  end
  
  def features_reply message
    return if OpenFlowSwitch.find_by_dpid(message.datapath_id)
    puts "controller gets switch #{message.datapath_id.to_hex}"

    OpenFlowSwitch.transaction do
      sw = OpenFlowSwitch.create(:dpid => message.datapath_id)
      
      message.ports.each do |trema_port|
        sw.open_flow_ports << OpenFlowPort.create(:no => trema_port.number)
      end
    end
  end

  def list_switches
    OpenFlowSwitch.all.each do |sw|
      puts "list switch #{sw.dpid}"
      sw.open_flow_ports.each do |port|
        puts "  port[#{port.no}]"
      end
    end
  end
end
