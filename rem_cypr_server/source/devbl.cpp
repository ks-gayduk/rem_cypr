
#include "devbl.h"

int get_dev_boards_list(vector<USBIP_DEV_LIST_ENTRY> &devs)
{
    FILE *fp;
    char  buf[SIZE_BUF];
    int   cnt;

    vector<string>               usb_all;
    vector<USBIP_DEV_LIST_ENTRY> usb_all_pars;
    vector<string>               usb_cypr;
    vector<string>               usb_cypr_pars;

    if (!(fp  = popen(CMD_USBIP_DEV_LIST, "r"))) return EXIT_FAILURE;
    cnt = fread(buf, 1, SIZE_BUF, fp);
    buf[cnt] = '\0';
    
    // Парсинг списка доступных для "расшаривания" устройств.
    char *p = strtok(buf, "\n");
    if (p)
    {
        usb_all.push_back(string(p));
    }
    while (p = strtok(NULL, "\n"))
    {
        usb_all.push_back(string(p));
    }

    // Построчный парсинг. Выделение пути и идентификаторов.
    for (int i = 0; i < usb_all.size(); i++)
    {
        USBIP_DEV_LIST_ENTRY udle;
        p = strtok((char*)(usb_all[i].c_str() + 9), " ");
        strcpy((char*)udle.path, p); // bus + ports
        p = strtok(NULL, " ");
        p++;
        p[9] = '\0';
        strcpy((char*)udle.id, p); // vendor id + product id
        udle.bind = 0;
        usb_all_pars.push_back(udle);
    }

    pclose(fp);
    
    // Получение списка отладочных плат.
    if (!(fp  = popen(CMD_CYPRS_DEV_LIST, "r"))) return EXIT_FAILURE;
    cnt = fread(buf, 1, SIZE_BUF, fp);
    buf[cnt] = '\0';
    
    // Парсинг списка отладочных плат.
    p = strtok(buf, "\n");
    if (p)
    {
        usb_cypr.push_back(string(p));
    }
    while (p = strtok(NULL, "\n"))
    {
        usb_cypr.push_back(string(p));
    }
    
    // Построчный парсинг. Выделение идентификаторов.
    for (int i = 0; i < usb_cypr.size(); i++)
    {
        p = strtok((char*)(usb_cypr[i].c_str() + 23), " ");
        usb_cypr_pars.push_back(string(p));
    }
    
    pclose(fp);
    
    // Фильтрация списка usb_all_pars. Отбираем только отладочные платы.
    for (int i = 0; i < usb_all_pars.size(); i++)
    {
        int f = 0;
        for (int j = 0; j < usb_cypr_pars.size(); j++)
        {
            if (!strcmp((char*)usb_cypr_pars[j].c_str(), (char*)usb_all_pars[i].id))
            {
                f = 1;
                break;
            }
        }
        
        if (f) devs.push_back(usb_all_pars[i]);
    }
    
    return EXIT_SUCCESS;
}

