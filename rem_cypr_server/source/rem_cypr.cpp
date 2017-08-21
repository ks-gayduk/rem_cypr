
#include "rem_cypr.h"

// volatile - потому что изменение возможно в обработчике сигналов.
volatile sig_atomic_t graceful_shutdown   = 0;
volatile sig_atomic_t hup_signal_caught   = 0;
int                   lock_file_desc      = -1;
int                   master_sock         = -1;
const int             port                = 5555;
const char           *lock_file_path      = "/run/rem_cypr.pid";
volatile int          max_files_desc      = 0;
volatile pid_t        daemon_pid;
const char           *app_name            = "rem_cypr";
struct sigaction      signal_action;
sigset_t              sig_mask;
int                   block_signals[]     = {SIGUSR2, SIGALRM, SIGPIPE, SIGTSTP,
                                             SIGPROF, SIGCHLD};
int                   handle_signals[]    = {SIGUSR1, SIGHUP, SIGTERM, SIGILL,
                                             SIGTRAP, SIGQUIT, SIGABRT, SIGIOT, 
                                             SIGBUS, SIGFPE, SIGSEGV, SIGSTKFLT,
                                             SIGCONT, SIGPWR, SIGSYS};
int                   bind_flags[3];

// Обычный процесс делает демоном.
int proc_to_daemon(void)
{
    char         pid_str[7];
    pid_t        cur_pid;
    int          lock_result;
    struct flock lock;
    
    // Макс. кол-во файлов, которое может открыть процесс.
    max_files_desc = sysconf(_SC_OPEN_MAX);

    // Порождение нового процесса.
    switch (cur_pid = fork ())
    {
        case 0: // Мы в дочернем процессе.
            break;
        case -1: // Мы в родительском, но возникла ошибка.
            fprintf(stderr, "Error: initial fork failed: %s\n", 
                    strerror(errno));
            return -1;
            break;
        default: // Мы в родительском, всё нормально. 
                 // Род. процесс завершается с кодом 0.
            exit(0);
            break;
    }

    daemon_pid = getpid();
    sprintf (pid_str, "%d\n", daemon_pid);
    openlog(app_name, LOG_NOWAIT, LOG_LOCAL0);
    if ((lock_file_desc = creat(lock_file_path, 0644)) < 0)
    {
        syslog(LOG_LOCAL0 | LOG_INFO, 
                "Couldn't open lock file, ERROR #%d", errno);
        return EXIT_FAILURE;
    }
    
    // Блокировка доступа к файлу PID.
    memset(&lock, 0, sizeof(lock));
    lock.l_type   = F_WRLCK;  // Блокировка разделения записи.
    lock.l_whence = SEEK_SET; // С начала файла.
    lock.l_start  = 0;
    lock.l_len    = 0;

    if ((lock_result = fcntl(lock_file_desc, F_SETLK, &lock)) < 0)
    {
        syslog(LOG_LOCAL0 | LOG_INFO, "Couldn't set lock to file %s, ERROR #%d",
                lock_file_path, errno);
    }
    
    // Записываем PID в файл.
    if (write(lock_file_desc, pid_str, strlen(pid_str)) <= 0)
    {
        syslog(LOG_LOCAL0 | LOG_INFO, 
                "Couldn't write PID to lock file, ERROR #%d", errno);
    }
    
    // Создание сеанса.
    // Тем самым демон оказывается "оторванным" от какого-либо терминала.
    if (setsid() < 0)
    {
        syslog(LOG_LOCAL0 | LOG_INFO, 
                "Couldn't get session ID (SID) from Kernel, ERROR #%d", errno);
        return EXIT_FAILURE;
    }
    
    if (chdir("/") < 0)
    {
        syslog(LOG_LOCAL0 | LOG_INFO, 
                "Couldn't change directory to /, ERROR #%d", errno);
        return EXIT_FAILURE;
    }

    syslog(LOG_LOCAL0 | LOG_INFO, "Started with PID #%d", daemon_pid);
    
    return EXIT_SUCCESS;
}

// Назначение своего обработчика сигналов.
int configure_signal_handlers(void)
{
    sigemptyset(&sig_mask); // Пустой набор сигналов.

    // Добавление сигналов из массива block_signals. 
    // Будут блокироваться на время выполнения обработчика.
    for (int i = 0; i < 6; i++)
    {
        sigaddset(&sig_mask, *(block_signals + i));
    }

    signal_action.sa_handler = handler;
    signal_action.sa_mask    = sig_mask;
    signal_action.sa_flags   = 0;
    sigaction(SIGUSR1, &signal_action, NULL);
    
    // Для сигналов из массива handle_signals задаем свой обработчик.
    for (int i = 0; i < 15; i++)    {
        sigaction(*(handle_signals + i), &signal_action, NULL);
    }
    
    return EXIT_SUCCESS;
}

// Обработчик сигналов.
void handler(int signal)
{
    syslog(LOG_LOCAL0 | LOG_INFO, "Received signal %d", signal);
    
    switch (signal)
    {
        case SIGUSR1: // Пользовательский сигнал. Корректная остановка.
            syslog(LOG_LOCAL0 | LOG_INFO, "Stopped gracefully");
            graceful_shutdown = 1;
            break;
        case SIGHUP: // Потеря связи с терминалом.
            syslog(LOG_LOCAL0 | LOG_INFO, "HUP signal caught");
            graceful_shutdown = hup_signal_caught = 1;
            break;
        case SIGTERM: // Сигнал завершения процесса.
            syslog (LOG_LOCAL0 | LOG_INFO, "Stopped");
            do_cleanup();
            unlink(lock_file_path);
            exit(EXIT_SUCCESS);
            break;
        default:
            #ifdef _GNU_SOURCE
            syslog(LOG_LOCAL0 | LOG_INFO, 
                    "Caught signal %s - exiting", strsignal(signal));
            #else
            syslog(LOG_LOCAL0 | LOG_INFO, "Caught signal %d - exiting", signal);
            #endif
            do_cleanup();
            unlink(lock_file_path);
            exit(0);
            break;
    }
}

// Освобождение ресурсов перед завершением.
int do_cleanup(void)
{
    for (int i = 0; i < max_files_desc; i++)
    {
        if (i != lock_file_desc)
        {
            close(i);
        }
    }

    closelog();
    close(master_sock);
}

// Связывание порта и адреса.
int bind_passive_sock(void)
{
    struct sockaddr_in sock_addr;
    int    sock, option_value;
    size_t option_length;
    
    memset(&sock_addr.sin_zero, 0, 8);
    
    sock_addr.sin_port        = htons(port);
    sock_addr.sin_family      = AF_INET;
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Со всеми лок. интерфейсами.
    
    // Создание сокета.
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        return -1;
    }
    
    option_value  = 1;
    option_length = sizeof(int);
    
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &option_value, option_length);
    
    // Присваивание сокету имени.
    if ((bind(sock, (struct sockaddr *)&sock_addr, 
            sizeof(struct sockaddr_in))) < 0)
    {
        return -1;
    }
    
    // Начинаем случать. Кол-во клиентов – максимальное.
    if ((listen(sock, SOMAXCONN)) < 0)
    {
        return -1;
    }
    
    master_sock = sock;
    
    return 0;
}

// Получение строки.
int read_line(const int sock, char *buffer, const size_t len)
{
    int     return_value = 0;
    ssize_t read_result;
    
    read_result = recv(sock, buffer, len, 0);
    
    if (read_result == len) return_value = 0;
    else return_value = -1;
    
    return return_value;
}

// Отправка строки.
int write_to_socket(const int sock, const char *buffer, const size_t len)
{
    ssize_t write_result;
    int     return_value = 0;
    
    write_result = send(sock, buffer, len, 0);
    
    if (write_result == len) return_value = 0;
    else return_value = -1;
    
    return return_value;
}

// Обработчик соединения.
int handler_connection(const int slave)
{
    char                         read_buffer[sizeof(REQUEST_PACK)];
    const size_t                 buffer_length = sizeof(REQUEST_PACK);
    int                          return_value;
    vector<USBIP_DEV_LIST_ENTRY> udb;
    RESPONSE_PACK                packet = {0};

    memset(read_buffer, 0, buffer_length);
    return_value = read_line(slave, read_buffer, buffer_length);

    if (return_value == 0)
    {
        switch (((PREQUEST_PACK)(&read_buffer))->code)
        {
            case req_dev_list:
                if (!get_dev_boards_list(udb))
                {
                    packet.code  = res_dev_list_succ;
                    packet.count = udb.size();
                    for (int i = 0; i < udb.size(); i++)
                    {
                        memcpy(&packet.dev_list[i], &udb[i], sizeof(USBIP_DEV_LIST_ENTRY));
                        packet.dev_list[i].bind = bind_flags[i];
                    }
                }
                else
                {
                    packet.code  = res_dev_list_fail;
                }
                break;
            case req_conn:
                if (!get_dev_boards_list(udb))
                {
                    int num = ((PREQUEST_PACK)(&read_buffer))->dev_num;
                    if (num < udb.size())
                    {
                        if (!bind_dev_board((char*)udb[num].path))
                        {
                            packet.code  = res_conn_succ;
                            bind_flags[num] = 1;
                        }
                        else
                        {
                            packet.code  = res_conn_fail;
                        }
                    }
                    else
                    {
                        packet.code  = res_conn_fail;
                    }
                }
                else
                {
                    packet.code  = res_conn_fail;
                }
                break;
            case req_disconn:
                if (!get_dev_boards_list(udb))
                {
                    int num = ((PREQUEST_PACK)(&read_buffer))->dev_num;
                    if (num < udb.size())
                    {
                        if (!unbind_dev_board((char*)udb[num].path))
                        {
                            packet.code  = res_disconn_succ;
                            bind_flags[num] = 0;
                        }
                        else
                        {
                            packet.code  = res_disconn_fail;
                        }
                    }
                    else
                    {
                        packet.code  = res_disconn_fail;
                    }
                }
                else
                {
                    packet.code  = res_disconn_fail;
                }
                break;
            default:
                break;
        }
        
        return_value = write_to_socket(slave, (char*)&packet, sizeof(packet));
    }

    return return_value;
}

// Установление соединения и обмен данными.
int accept_connection(const int master)
{
    int                slave, return_value = 0;
    struct sockaddr_in client;
    socklen_t          client_length;
    
    client_length = sizeof(client);
    // Общение будет идти через slave.
    // Пока идет установление связи, др. запросы блокируются.
    slave = accept(master, (struct sockaddr *)&client, &client_length);

    if (slave < 0)
    {
        if (errno != EINTR)
        {
            syslog (LOG_LOCAL0 | LOG_INFO, "accept() failed: %m\n");
            return_value = -1;
        }
    }
    else
    {
        // Обработка запроса.
        return_value = handler_connection(slave);
        // Приняли, ответили, закрыли.
        close(slave);
    }

    return return_value;
}

// Подключение устройства по пути sysfs.
int bind_dev_board(char *path)
{
    FILE *fp;
    char  buf[SIZE_BUF];
    int   cnt;

    sprintf(buf, "usbip bind -b %s | grep complete", path);
    if (!(fp  = popen(buf, "r"))) return EXIT_FAILURE;
    cnt = fread(buf, 1, SIZE_BUF, fp);
    if (cnt == 0)
    {
        pclose(fp);
        return EXIT_FAILURE;
    }

    pclose(fp);
    return EXIT_SUCCESS;
}

// Отключение устройства по пути sysfs.
int unbind_dev_board(char *path)
{
    FILE *fp;
    char  buf[SIZE_BUF];
    int   cnt;

    sprintf(buf, "usbip unbind -b %s | grep complete", path);
    if (!(fp  = popen(buf, "r"))) return EXIT_FAILURE;
    cnt = fread(buf, 1, SIZE_BUF, fp);
    if (cnt == 0)
    {
        pclose(fp);
        return EXIT_FAILURE;
    }

    pclose(fp);
    return EXIT_SUCCESS;
}
