# == Schema Information
#
# Table name: conversations
#
#  id         :integer          not null, primary key
#  to         :string(255)
#  from       :string(255)
#  subject    :string(255)
#  mailbox_id :integer
#  created_at :datetime
#  updated_at :datetime
#

class Conversation < ActiveRecord::Base
	has_many :messages
	belongs_to :mailbox

  serialize :addresses
end
