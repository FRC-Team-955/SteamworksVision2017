#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <ostream>

#define BUFFER_SIZE 256

//Simple single-connection networking for limited use cases. Pretty small though.
namespace Networking {
	class Server {
		private:
			int server_port;

			int server_socket_file_descriptor;
			int client_connect_socket_file_descriptor;

			struct sockaddr_in server_address;
			struct sockaddr_in client_address;

			socklen_t client_address_struct_length;

			std::vector<char> message;

		public:
			Server(int port);

			void WaitForClientConnection ();

			int WaitForClientMessage (std::ostream* out_stream);

			void SendClientMessage (const char* stream_buffer);
			
	};
}
