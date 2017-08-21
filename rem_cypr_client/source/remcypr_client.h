
#ifndef _REMCYPR_CLIENT_
#define _REMCYPR_CLIENT_

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <iostream>

#define PORT       5555

#pragma pack(push,1)
typedef struct _USBIP_DEV_LIST_ENTRY
{
    unsigned char path[6];
    unsigned char id[10];
    unsigned char bind;
} USBIP_DEV_LIST_ENTRY, *PUSBIP_DEV_LIST_ENTRY;
#pragma pack(pop)

// ����� �������.
#pragma pack(push,1)
typedef struct _REQUEST_PACK
{
    unsigned char code;
    unsigned char dev_num;
    unsigned char reserved[51];
} REQUEST_PACK, *PREQUEST_PACK;
#pragma pack(pop)

// ����� ������.
#pragma pack(push,1)
typedef struct _RESPONSE_PACK
{
    unsigned char        code;
    unsigned char        count;
    USBIP_DEV_LIST_ENTRY dev_list[3];
} RESPONSE_PACK, *PRESPONSE_PACK;
#pragma pack(pop)

// ���� ��������.
enum REQUEST_CODE
{
    req_dev_list = 0x00, // ������ ���������� ����.
    req_conn     = 0x01, // �����������.
    req_disconn  = 0x02  // ����������.
};

// ���� �������.
enum RESPONCE_CODE
{
    res_dev_list_succ = 0x03, // ������ ���������� ���� - �������.
    res_conn_succ     = 0x04, // ����������� - �������.
    res_disconn_succ  = 0x05, // ���������� - �������.
    res_dev_list_fail = 0x06, // ������ ���������� ���� - ������.
    res_conn_fail     = 0x07, // ����������� - ������.
    res_disconn_fail  = 0x08  // ���������� - ������.
};

// ��������� ������ ���������.
int update_dev_list(const char *ip, RESPONSE_PACK &res_pack);

// ����������� � ���������� ����������.
int bind_rem_dev(const char *ip, const int num, RESPONSE_PACK &res_pack);

// ���������� �� ���������� ����������.
int unbind_rem_dev(const char *ip, const int num, RESPONSE_PACK &res_pack);

#endif // _REMCYPR_CLIENT_
