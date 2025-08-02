#include <QCoreApplication>
#include "CrashTryCatch.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    s_try{
        int* a = nullptr;
        int b = *a;
    }
    s_catch(crash_error,e)
    {
        qDebug() << e.what();
    };

    return a.exec();
}
