# == Schema Information
#
# Table name: mailboxes
#
#  id         :integer          not null, primary key
#  user_id    :integer
#  created_at :datetime
#  updated_at :datetime
#

class Mailbox < ActiveRecord::Base
	has_many :conversations
	belongs_to :user
end
