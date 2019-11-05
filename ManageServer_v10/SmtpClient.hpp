#pragma once
//****************************************************************************
//*                     include
//****************************************************************************
#include "framework.h"

//****************************************************************************
//*                     SmtpClient
//****************************************************************************
class SmtpClient {
public:
	//****************************************************************************
	//*                     read_socket
	//****************************************************************************
	inline void read_socket(SOCKET& sock) {
		int size_recv, total_size = 0;
		char chunk[BUFSIZ];
		char recvbuf[BUFSIZ] = "";

		memset(chunk, 0, BUFSIZ);  //clear the variable
		if ((size_recv = recv(sock, chunk, BUFSIZ, 0)) < 0)
			return;
		else
		{
			total_size += size_recv;
			strcat_s(recvbuf, BUFSIZ, chunk);
		}
		//std::cout << recvbuf;
	}

	//****************************************************************************
	//*                     read_socket_ssl
	//****************************************************************************
	inline void read_socket_ssl(SSL* ssl) {
		int size_recv, total_size = 0;
		char chunk[BUFSIZ];
		char recvbuf[BUFSIZ] = "";

		memset(chunk, 0, BUFSIZ);  //clear the variable
		if ((size_recv = SSL_read(ssl, chunk, BUFSIZ)) < 0)
			return;
		else
		{
			total_size += size_recv;
			chunk[total_size] = '\0';
			strcat_s(recvbuf, BUFSIZ, chunk);
			//std::cout << recvbuf;
		}
	}

	//****************************************************************************
	//*                     write_socket
	//****************************************************************************
	inline void write_socket(SOCKET& sock, const std::string& str) {
		char* buf = const_cast<char*>(str.c_str());
		int sended = 0;
		int all = strlen(buf);

		//std::cout << str;
		while (all > 0) {
			sended = send(sock, buf, all, 0);
			*buf += sended;
			all -= sended;
		}
	}

	//****************************************************************************
	//*                     write_socket_ssl
	//****************************************************************************
	inline void write_socket_ssl(SSL* ssl, const std::string& str) {
		char* buf = const_cast<char*>(str.c_str());
		int sended = 0;

		//std::cout << str;
		sended = SSL_write(ssl, buf, strlen(buf));
	}

	//****************************************************************************
	//*                     smtp_client
	//****************************************************************************
	int smtp_client(const std::string& mail_from
		, const std::string& rcpt_to
		, const std::string& from
		, const std::string& to
		, const std::string& subject
		, const std::string& data
	)
	{
		WSADATA wsaData = { 0 };
		int iResult = 0;

		// initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0)
			return iResult;

		SOCKET sock = INVALID_SOCKET;
		int iFamily = AF_INET;
		int iType = SOCK_STREAM;
		int iProtocol = 0;

		// initialize socket
		sock = socket(iFamily, iType, iProtocol);
		if (sock == INVALID_SOCKET)
			// socket function failed with error 
			return sock; // an INVALID_SOCKET value is returned

		// connect to server
		struct sockaddr_in server;
		char host_id[] = "74.125.142.109";
		if (inet_addr(host_id) != INADDR_NONE)
			server.sin_addr.s_addr = inet_addr(host_id);
		server.sin_family = AF_INET;
		server.sin_port = htons(587);
		if (connect(sock, (struct sockaddr*) & server, sizeof server) == -1)
			// can't connect to server
			return -1;

		read_socket(sock);
		write_socket(sock, "EHLO smtp.gmail.com\r\n");
		read_socket(sock);
		write_socket(sock, "STARTTLS\r\n");
		read_socket(sock);

		// switch over to the Secure Socket Layer (SSL)*/
		SSL* ssl = nullptr;
		SSL_CTX* ctx;

		//Associate socket to SLL
		SSL_library_init();
		SSL_load_error_strings();
		ctx = SSL_CTX_new(TLSv1_client_method());
		ssl = SSL_new(ctx);
		SSL_set_fd(ssl, sock);

		// reconnect with SSL
		SSL_connect(ssl);
		write_socket_ssl(ssl, "EHLO smtp.gmail.com\r\n");
		read_socket_ssl(ssl);
		write_socket_ssl(ssl, "auth plain AGtsYXNpbmdzbXRwQGdtYWlsLmNvbQBwcHJ5cGRudGZxcWlpc3Ru\r\n");
		read_socket_ssl(ssl);

		// from/to part
		write_socket_ssl(ssl, "MAIL FROM:<" + mail_from + ">\r\n");
		read_socket_ssl(ssl);
		write_socket_ssl(ssl, "RCPT TO:<" + rcpt_to + ">\r\n");
		read_socket_ssl(ssl);

		// message data
		write_socket_ssl(ssl, "DATA\r\n");
		write_socket_ssl(ssl, "FROM:<" + from + ">\r\n");
		write_socket_ssl(ssl, "TO:<" + to + ">\r\n");
		write_socket_ssl(ssl, "SUBJECT:<" + subject + ">\r\n");
		write_socket_ssl(ssl, data + "\r\n");
		write_socket_ssl(ssl, "\r\n");
		write_socket_ssl(ssl, "\r\n");
		write_socket_ssl(ssl, ".\r\n");

		// start sending message to recipient
		write_socket_ssl(ssl, "QUIT\r\n");
		read_socket_ssl(ssl);

		return 0;
	}
};