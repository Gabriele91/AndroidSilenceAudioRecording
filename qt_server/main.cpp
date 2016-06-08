#include "q_android_silence_audio_recording.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QDebug>

int main(int argc, char *argv[])
{
    //init qt app
    QApplication a(argc, argv);
    // load style
    QFile f(":qdarkstyle/style.qss");
    if (!f.exists())
    {
        printf("Unable to set stylesheet, file not found\n");
    }
    else
    {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }
    //alloc and init window
    q_android_silence_audio_recording w;
    w.show();
    //execute
    return a.exec();
}
