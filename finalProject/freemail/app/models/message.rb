# == Schema Information
#
# Table name: messages
#
#  id              :integer          not null, primary key
#  body            :string(255)
#  conversation_id :integer
#  created_at      :datetime
#  updated_at      :datetime
#

class Message < ActiveRecord::Base
	serialize :body
	belongs_to :conversation
end
