#include <QApplication>
#include <QIcon>
#include "FMFD.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    a.setWindowIcon(QIcon("./resource_files/icons/icon3.png")); // 如果用 Qt 资源系统

    FMFD w;
    w.show();
    return a.exec();
}