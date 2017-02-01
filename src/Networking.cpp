#include <Networking.hpp>

//http://www.linuxhowtos.org/C_C++/socket.htm
Networking::Server::Server (int server_port) {
	this->server_port = server_port;

	//Create the socket, returning a unix file descriptor for the socket
	server_socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);

	if (server_socket_file_descriptor < 0) {
		std::cerr << "Socket creation error" << std::endl;
	}

	//Set attributes in the address field
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(server_port); //Set the port, but in the correct byte order
	server_address.sin_addr.s_addr = INADDR_ANY; //Set the address of the host... Oooh this is cool I bet I can spoof it

	//http://stackoverflow.com/questions/5592747/bind-error-while-recreating-socket
	int yes=1;

	if (setsockopt(server_socket_file_descriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		std::cerr << "Cannot set socket binding option" << std::endl;
		exit(1);
	}

	if (bind(server_socket_file_descriptor, (struct sockaddr *) &server_address, sizeof(sockaddr)) < 0) {
		std::cerr << "Server socket binding error" << std::endl;
		exit(-1);
	}

	listen(server_socket_file_descriptor, 5);

	client_address_struct_length = sizeof(client_address);

}

void Networking::Server::WaitForClientConnection() {
	//close(client_connect_socket_file_descriptor);
	client_connect_socket_file_descriptor = accept(server_socket_file_descriptor, (struct sockaddr *) &client_address, &client_address_struct_length);

	if (client_connect_socket_file_descriptor < 0) {
		std::cerr << "Socket accept error" << std::endl;
		exit(-1);
	}

	std::cout << "Socket connected! Communication may proceed." << std::endl;
}

int Networking::Server::WaitForClientMessage (std::ostream* out_stream) {
	//[Googling problems intensifies]
	char errbuf;
	int errorcode = recv(client_connect_socket_file_descriptor, &errbuf, 1, MSG_PEEK); 
	std::string message = "";
	size_t message_read_length;
	char buffer[BUFFER_SIZE];

	do {
		bzero(buffer, BUFFER_SIZE); //This is very important. Otherwise you get garbage (dirty mem!)
		message_read_length = read(client_connect_socket_file_descriptor, buffer, BUFFER_SIZE - 1); 
		*out_stream << buffer;
	} while (message_read_length == BUFFER_SIZE - 1 && client_connect_socket_file_descriptor >= 0); //Keep reading until the message size is less than the buffer size (It's finished)
	return errorcode;
}

//TODO: Change this to a stream input instead!
void Networking::Server::SendClientMessage(const char* stream_buffer) {
		write(client_connect_socket_file_descriptor, stream_buffer, strlen(stream_buffer));
}
