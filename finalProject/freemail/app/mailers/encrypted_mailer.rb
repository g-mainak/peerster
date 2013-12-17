class EncryptedMailer < ActionMailer::Base

    class <<self
      def smtp_settings
        options = YAML.load_file("config/mailers.yml")[Rails.env]
        @@smtp_settings = {
          :address => options["address"],
          :port => options["port"],
          :domain => options["domain"],
          :authentication => options["authentication"],
          :tls => options["tls"]
        }
      end
    end

  def create(mailbox, options)
    if options[:encrypt]
      key = mailbox.user.contacts.find_by(email: options[:to]).key
      GPGME::Key.import(key)
      crypto = GPGME::Crypto.new(armor: true)
      d = crypto.encrypt options[:body], recipients: options[:to], always_trust: true
      m = mail(to: options[:to], from: mailbox.email, 
      subject: options[:subject],  body: d.read)
    else
      m = mail(to: options[:to], from: mailbox.email, 
        subject: options[:subject], body: options[:body])
    end
    m.delivery_method.settings[:user_name] = mailbox.email
    m.delivery_method.settings[:password] = mailbox.password
    m
  end
end