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

# ActiveRecord data model as OpenFlowSwitch
class OpenFlowSwitch < ActiveRecord::Base
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
  end
  
  def switch_ready dpid
    return if OpenFlowSwitch.find_by_dpid(dpid)
    OpenFlowSwitch.create(:dpid => dpid)
    puts "controller gets switch #{dpid.to_hex}"
  end

  def list_switches
    OpenFlowSwitch.all.each do |sw|
      puts "list switch #{sw.dpid}"
    end
  end
end
