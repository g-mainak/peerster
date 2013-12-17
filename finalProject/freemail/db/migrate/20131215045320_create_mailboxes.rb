class CreateMailboxes < ActiveRecord::Migration
  def change
    create_table :mailboxes do |t|
    	t.string :email
      t.string :password
    	t.references :user
    	t.timestamps
    end
  end
end
