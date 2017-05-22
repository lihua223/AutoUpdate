#include "dialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSharedMemory *shareMem = new QSharedMemory(QString("AutoUpdate"));

    /* if the sharedmemory has not been created, it returns false, otherwise true.
     * But if the application exit unexpectedly, the sharedmemory will not detach.
     * So, we try twice.
     */
    volatile short i = 2;
    while (i--)
    {
        if (shareMem->attach(QSharedMemory::ReadOnly)) /* no need to lock, bcs it's read only */
        {
            shareMem->detach();
        }
    }

    if (shareMem->create(1))
    {
        a.setQuitOnLastWindowClosed(false);

        Dialog w;
        w.hide();
        //QMessageBox::about(NULL,"111","11");
        return a.exec();
    }
    else
    {
        //QMessageBox::about(NULL,"222","22");
        exit(0);
    }
}
