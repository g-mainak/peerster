class CreateConversations < ActiveRecord::Migration
  def change
    create_table :conversations do |t|
      t.string :addresses
      t.string :subject

      t.references :mailbox
      t.timestamps
    end
  end
end
