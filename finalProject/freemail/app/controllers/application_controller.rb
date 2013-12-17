
class ApplicationController < ActionController::Base
  # Prevent CSRF attacks by raising an exception.
  # For APIs, you may want to use :null_session instead.
  protect_from_forgery except: :incoming_email

  def incoming_email
  	message = Mail.read_from_string(request.raw_post)
  	mailbox = Mailbox.find_by(email: message.to)
  	save_mail(mailbox, message)
  	render nothing: true
  end

  private
    def processed_subject(string)
      if r = string.match(/((\[)?(Re|Fwd?):?(\])? *:? *)+/i)
        r.post_match
      else
        string
      end
    end

  protected
    def current_user
      @current_user ||= User.find_by_id(session[:user_id])
    end

    def current_user=(user)
      @current_user = user
      session[:user_id] = user.nil? ? user : user.id
    end

    def save_mail(mailbox, message)
      conversation = mailbox.conversations.find_or_create_by(subject: processed_subject(message.subject))
      conversation.addresses ||= []
      conversation.addresses << message.from[0]
      conversation.addresses << message.to[0]
      conversation.addresses = conversation.addresses.uniq
      conversation.save!
      conversation.messages.create(body: message)
    end
end
