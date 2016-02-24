#ifndef MYACCOUNT_H
#define MYACCOUNT_H

#include <QDialog>
#include "login.h"

namespace Ui {
class MyAccount;
}

class MyAccount : public QDialog
{
    Q_OBJECT

public:

    explicit MyAccount(QWidget *parent = 0);
    ~MyAccount();

private slots:
    void on_pushButton_clicked();

    //void on_pushButton_2_clicked();

    void on_button_signOut_clicked();

    void on_pushbutton_update_clicked();

    void on_toolButton_clicked();
    
    void on_label_status_linkActivated(const QString &link);

    void on_pushButton_dev_clicked();

    /*Digital Clock*/
    void showTime();

    void on_pushButton_dev_2_clicked();

    void on_pushButton_password_clicked();

private:
    Ui::MyAccount *ui;
};

#endif // MYACCOUNT_H
