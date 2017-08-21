
#include "remcypr_client.h"

// Получение списка устройств.
int update_dev_list(const char *ip, RESPONSE_PACK &res_pack)
{
	// Регистрация приложения для получения доступа к интерфейсу Windows Sockets.
	WSADATA wsa;
	if (WSAStartup(WINSOCK_VERSION, &wsa)) return -1;

	// Создание сокета.
	SOCKET sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
	{
		WSACleanup();
		return -1;
	}

	// Именование сокета.
	sockaddr_in dest_addr;
	memset(&dest_addr, 0, sizeof(sockaddr_in));
    dest_addr.sin_family      = AF_INET;
    dest_addr.sin_port        = htons(PORT);
    dest_addr.sin_addr.s_addr = inet_addr(ip);

	// Подключение к серверу.
	if (connect(sock, (sockaddr*)&dest_addr, sizeof(dest_addr)))
	{
		closesocket(sock);
		WSACleanup();
		return -1;
	}

	// Отправка запроса.
	REQUEST_PACK req_pack;
	req_pack.code = req_dev_list;
	int nsize = send(sock, (char*)&req_pack, sizeof(req_pack), 0);
	if (nsize != sizeof(req_pack))
	{
		closesocket(sock);
		WSACleanup();
		return -1;
	}

	// Получение ответа.
	nsize = recv(sock, (char*)&res_pack, sizeof(res_pack), 0);
	if (nsize != sizeof(res_pack))
	{
		closesocket(sock);
		WSACleanup();
		return -1;
	}

	if (res_pack.code == res_dev_list_fail)
	{
		closesocket(sock);
		WSACleanup();
		return -1;
	}

	closesocket(sock);
    WSACleanup();

	return 0;
}

// Подключение к выбранному устройству.
int bind_rem_dev(const char *ip, const int num, RESPONSE_PACK &res_pack)
{
	// Регистрация приложения для получения доступа к интерфейсу Windows Sockets.
	WSADATA wsa;
	if (WSAStartup(WINSOCK_VERSION, &wsa)) return -1;

	// Создание сокета.
	SOCKET sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
	{
		WSACleanup();
		return -1;
	}

	// Именование сокета.
	sockaddr_in dest_addr;
	memset(&dest_addr, 0, sizeof(sockaddr_in));
    dest_addr.sin_family      = AF_INET;
    dest_addr.sin_port        = htons(PORT);
    dest_addr.sin_addr.s_addr = inet_addr(ip);

	// Подключение к серверу.
	if (connect(sock, (sockaddr*)&dest_addr, sizeof(dest_addr)))
	{
		closesocket(sock);
		WSACleanup();
		return -1;
	}

	// Отправка запроса.
	REQUEST_PACK req_pack;
	req_pack.code    = req_conn;
	req_pack.dev_num = num;
	int nsize = send(sock, (char*)&req_pack, sizeof(req_pack), 0);
	if (nsize != sizeof(req_pack))
	{
		closesocket(sock);
		WSACleanup();
		return -1;
	}

	// Получение ответа.
	nsize = recv(sock, (char*)&res_pack, sizeof(res_pack), 0);
	if (nsize != sizeof(res_pack))
	{
		closesocket(sock);
		WSACleanup();
		return -1;
	}

	if (res_pack.code == res_conn_fail)
	{
		closesocket(sock);
		WSACleanup();
		return -1;
	}

	closesocket(sock);
    WSACleanup();

	return 0;
}

// Отключение от выбранного устройства.
int unbind_rem_dev(const char *ip, const int num, RESPONSE_PACK &res_pack)
{
	// Регистрация приложения для получения доступа к интерфейсу Windows Sockets.
	WSADATA wsa;
	if (WSAStartup(WINSOCK_VERSION, &wsa)) return -1;

	// Создание сокета.
	SOCKET sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
	{
		WSACleanup();
		return -1;
	}

	// Именование сокета.
	sockaddr_in dest_addr;
	memset(&dest_addr, 0, sizeof(sockaddr_in));
    dest_addr.sin_family      = AF_INET;
    dest_addr.sin_port        = htons(PORT);
    dest_addr.sin_addr.s_addr = inet_addr(ip);

	// Подключение к серверу.
	if (connect(sock, (sockaddr*)&dest_addr, sizeof(dest_addr)))
	{
		closesocket(sock);
		WSACleanup();
		return -1;
	}

	// Отправка запроса.
	REQUEST_PACK req_pack;
	req_pack.code    = req_disconn;
	req_pack.dev_num = num;
	int nsize = send(sock, (char*)&req_pack, sizeof(req_pack), 0);
	if (nsize != sizeof(req_pack))
	{
		closesocket(sock);
		WSACleanup();
		return -1;
	}

	// Получение ответа.
	nsize = recv(sock, (char*)&res_pack, sizeof(res_pack), 0);
	if (nsize != sizeof(res_pack))
	{
		closesocket(sock);
		WSACleanup();
		return -1;
	}

	if (res_pack.code == res_disconn_fail)
	{
		closesocket(sock);
		WSACleanup();
		return -1;
	}

	closesocket(sock);
    WSACleanup();

	return 0;
}


