#ifndef ATT_THREAD_H
#define ATT_THREAD_H

#include <QThread>
#include <stdio.h>
#include <string.h>

class AttachThread : public QThread
{

public:
    void run ();
};

#endif // ATT_THREAD_H
