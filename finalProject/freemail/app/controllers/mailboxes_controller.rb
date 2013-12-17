class MailboxesController < ApplicationController
	after_action :delete_keys, only: [:send_mail]
  def index
		@mailboxes = current_user.mailboxes
    @user = current_user
	end

	def show
		@mailbox = Mailbox.find params[:id]
		@conversations = @mailbox.conversations
	end

  def send_mail
    current_mailbox = Mailbox.find params[:id]
    mail = EncryptedMailer.create(current_mailbox, params)
    save_mail(current_mailbox, mail)
    mail.deliver
    redirect_to current_mailbox, notice: "Message sent"
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
