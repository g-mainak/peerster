class UsersController < ApplicationController
	def new
		@user = User.new
		@mailbox  = @user.mailboxes.build
	end

	def create
		@user = User.create(user_params)
		self.current_user= @user
		redirect_to mailboxes_path, notice: "Welcome #{current_user.name}!"
	end

	def sign_in
		if (@mailbox = Mailbox.find_by(email: params[:email], password: params[:password]))
			self.current_user= @mailbox.user
			redirect_to mailboxes_path, notice: "Welcome back #{current_user.name}!"
		else
			redirect_to :root
		end
	end

	def show
	end

	def destroy
	end

	def edit
	end

	def update
	end

	def add_private_key
		current_user.key= params[:key]
		current_user.save!
		redirect_to mailboxes_path
	end

	def add_contact
		current_user.contacts.create(name: params[:name],
			email: params[:email],
			key: params[:key])
		redirect_to mailboxes_path
	end

	private
		def user_params
			params.require(:user).permit(:name, mailboxes_attributes: [:email, :password])
		end

		def current_user
      @current_user ||= User.find_by_id(session[:user_id])
    end
end
