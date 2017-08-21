
#include "rem_cypr.h"

extern const char           *lock_file_path;
extern int                   bind_flags[3];
extern int                   master_sock;
extern volatile sig_atomic_t graceful_shutdown;
extern volatile sig_atomic_t hup_signal_caught;

using namespace std;

int main(int argc, char** argv)
{
    int   result;
    
    // Делаем из процесса демона.
    if((result = proc_to_daemon()) < 0)
    {
        perror("Failed to become daemon process");
        unlink(lock_file_path);
        exit(result);
    }

    // Настраиваем обработку сигналов.
    if ((result = configure_signal_handlers()) < 0)
    {
        syslog(LOG_LOCAL0 | LOG_INFO, "Failed to configure signal handlers.");
        unlink(lock_file_path);
        exit(result);
    }

    // Открываем порт на прослушивание.
    if ((result = bind_passive_sock()) < 0)
    {
        syslog(LOG_LOCAL0 | LOG_INFO, "Failed to bind passive socket.");
        unlink(lock_file_path);
        exit(result);
    }

    memset(&bind_flags, 0, sizeof(bind_flags));
    
    // Цикл последовательной обработки запросов.
    do
    {
        if ((accept_connection(master_sock)) < 0)
        {
            // Если причина ошибки – не сигнал, прервавший сис. вызов accept.
            if (errno != EINTR)
            {
                syslog (LOG_LOCAL0 | LOG_INFO, 
                        "accept_connection failed, ERROR #%d", errno);
                syslog (LOG_LOCAL0 | LOG_INFO, "Stopped");
                do_cleanup();
                unlink(lock_file_path);
                exit(result);
            }
        }
        
        if ((graceful_shutdown == 1) && (hup_signal_caught == 0))
        {
            break;
        }
        
        graceful_shutdown = hup_signal_caught = 0;
    } while (1);

    // Закрываем дескрипторы, удаляем pid-файл.
    do_cleanup();
    unlink(lock_file_path);
    return 0;
}
