# == Schema Information
#
# Table name: contacts
#
#  id         :integer          not null, primary key
#  name       :string(255)
#  key        :string(255)
#  user_id    :integer
#  created_at :datetime
#  updated_at :datetime
#

class Contact < ActiveRecord::Base
	belongs_to :user
end
