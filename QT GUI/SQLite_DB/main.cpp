#include "login.h"
#include <QApplication>
#include<QSplashScreen>
#include<QTimer>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSplashScreen *splash = new QSplashScreen;
    splash->setPixmap(QPixmap("/media/shreyas/DATA/Photography/Shortlisted/splashscreen.png"));
    splash->show();

    Login w;

    //Splash Screen will appear for 2.5sec (2500msec) and then start the application
    QTimer::singleShot(2500,splash,SLOT(close()));
    QTimer::singleShot(2500,&w,SLOT(show()));

    return a.exec();
}
