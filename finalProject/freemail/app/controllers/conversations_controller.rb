class ConversationsController < ApplicationController
  require 'encrypted_mailer'

  after_action :delete_keys, only: [:show]

	def show
		@conversation = Conversation.find params[:id]
    @times = @conversation.messages.map(&:created_at)
		@messages =  @conversation.messages.map(&:body)
    @reply_address = @messages.last.From

    key = current_user.key
    GPGME::Key.import(key)
    @crypto = GPGME::Crypto.new
	end

  private
    def current_user
      @current_user ||= User.find_by_id(session[:user_id])
    end

    def delete_keys
      GPGME::Key.find(:secret).each{ |i| i.delete!(allow_secret: true)}
      GPGME::Key.find(:public).each{ |i| i.delete!(allow_secret: true)}
    end
end