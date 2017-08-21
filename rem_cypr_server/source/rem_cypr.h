
#ifndef REM_CYPR_H
#define REM_CYPR_H

#include <cstdlib>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>

#include "devbl.h"

// Пакет запроса.
#pragma pack(push,1)
typedef struct _REQUEST_PACK
{
    unsigned char code;
    unsigned char dev_num;
    unsigned char reserved[51];
} REQUEST_PACK, *PREQUEST_PACK;
#pragma pack(pop)

// Пакет ответа.
#pragma pack(push,1)
typedef struct _RESPONSE_PACK
{
    unsigned char        code;
    unsigned char        count;
    USBIP_DEV_LIST_ENTRY dev_list[3];
} RESPONSE_PACK, *PRESPONSE_PACK;
#pragma pack(pop)

// Коды запросов.
enum REQUEST_CODE
{
    req_dev_list = 0x00, // Список отладочных плат.
    req_conn     = 0x01, // Подключение.
    req_disconn  = 0x02  // Отключение.
};

// Коды ответов.
enum RESPONCE_CODE
{
    res_dev_list_succ = 0x03, // Список отладочных плат - успешно.
    res_conn_succ     = 0x04, // Подключение - успешно.
    res_disconn_succ  = 0x05, // Отключение - успешно.
    res_dev_list_fail = 0x06, // Список отладочных плат - ошибка.
    res_conn_fail     = 0x07, // Подключение - ошибка.
    res_disconn_fail  = 0x08  // Отключение - ошибка.
};

int  proc_to_daemon(void);
int  do_cleanup(void);
void handler(int signal);
int  configure_signal_handlers(void);
int  bind_passive_sock(void);
int  read_line(const int sock, char *buffer, const size_t len);
int  write_to_socket(const int sock, const char *buffer, const size_t len);
int  handler_connection(const int slave);
int  accept_connection(const int master);
int  bind_dev_board(char *path);
int  unbind_dev_board(char *path);

#endif /* REM_CYPR_H */

