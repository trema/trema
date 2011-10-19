#
# Monkey patches for Ruby's built-in classes.
#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
#
# Copyright (C) 2008-2011 NEC Corporation
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


class Integer
  def to_hex
    "%#x" % self
  end


  def unsigned_8bit?
    unsigned_bit?( 8 ).call
  end


  def unsigned_16bit?
    unsigned_bit?( 16 ).call
  end


  def unsigned_32bit?
    unsigned_bit?( 32 ).call
  end


  ################################################################################
  private
  ################################################################################
  def unsigned_bit? number
    lambda { ( 0 <= self ) and ( self < 2 ** number ) }
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
