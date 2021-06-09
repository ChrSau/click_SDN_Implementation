#include "TCPClient.hh"

TCPClient::TCPClient()
{
	this->sock = -1;
	this->port = 0;
	this->address = "";
}

bool TCPClient::setup(std::string address, int port)
{
	if (this->sock == -1)
	{
		this->sock = socket(AF_INET, SOCK_STREAM, 0);
		if (this->sock == -1)
		{
			std::cout << "Could not create socket" << std::endl;
		}
	}
	if (inet_addr(address.c_str()) == -1)
	{
		struct hostent *he;
		// struct in_addr **addr_list;
		if ((he = gethostbyname(address.c_str())) == NULL)
		{
			herror("gethostbyname");
			std::cout << "Failed to resolve hostname\n";
			return false;
		}
		// addr_list = (struct in_addr **)he->h_addr_list;
		// for (int i = 0; addr_list[i] != NULL; i++)
		// {
		// 	server.sin_addr = *addr_list[i];
		// 	break;
		// }
		std::memcpy((char *)&server.sin_addr.s_addr, (char *)he->h_addr, he->h_length);
	}
	else
	{
		server.sin_addr.s_addr = inet_addr(address.c_str());
	}
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (connect(this->sock, (struct sockaddr *)&this->server, sizeof(this->server)) < 0)
	{
		perror("connect failed. Error");
		return false;
	}

	// fcntl(sock, F_SETFL, O_NONBLOCK);
	// fcntl(sock, F_SETFD, FD_CLOEXEC);
	return true;
}

bool TCPClient::Send(uint8_t *data, size_t length)
{
	if (data != nullptr && data != NULL && length > 0)
	{
		OF::Functions::printHexAndChar(data, length, "OutPacket: ");
		if (sock != -1)
		{
			if (send(sock, data, length, 0) < 0)
			{
				std::cout << "Send failed : " << std::endl;
				return false;
			}
		}
		else
			return false;
		return true;
	}
	else
	{
		return false;
	}
}

bool TCPClient::receive(uint8_t *buffer, int size, uint32_t *_len)
{
	// int n;

	*_len = read(this->sock, buffer, size);
	if (*_len < 0)
	{
		std::cout << "receive failed!" << std::endl;
		std::cout << strerror(errno) << "\n";
		return false;
	}

	if (*_len == 0)
	{

		// if (errno != EAGAIN)
		std::cout << strerror(errno) << "\n";
		// else
		std::cout << "Connection Closed\n";

		this->exit();

		return false;
	}

	// *_len = n;

	OF::Functions::printHexAndChar(buffer, *_len, "InPacket: ");

	return true;
}

void TCPClient::exit()
{
	if (this->sock != -1)
	{
		close(this->sock);
		this->sock = -1;
	}
}
