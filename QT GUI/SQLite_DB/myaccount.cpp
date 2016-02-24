#include "myaccount.h"
#include "ui_myaccount.h"
#include <login.h>
#include <QMessageBox>

/*To open the web page*/
#include <QDesktopServices>
#include <QUrl>

/*Digial Clock*/
#include<QTimer>
#include<QDateTime>
#include<QDate>

extern  QString username, password, status, userEmail;/*used as Global variable from the login window to be used in my account window*/

MyAccount::MyAccount(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MyAccount)
{
    /*To show the current Time and Date*/
    ui->setupUi(this);
    QTimer *timer=new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(showTime()));
    timer->start();

    QDate dateTime = QDate::currentDate();
    QString dateTime_text = dateTime.toString();
    ui->label_time->setText(dateTime_text);

    /*Check the Database connection*/
    Login conn;
        if(!conn.connOpen())
            ui->label_sec_status->setText("Failed to open the Database");
        else
            ui->label_sec_status->setText("Connected to Database");


   //Custom Name for each logged in user
   ui->label_username->setText("Welcome " + username + userEmail);

}
//Show current time in HH:MM format
void MyAccount::showTime()
{
    QTime time = QTime::currentTime();

    QString time_text=time.toString("hh : mm");
    /* tick timer every second */
    if ((time.second() % 2) == 0)
    {
        time_text[3] = ' ';
    }
    ui->label_clock->setText(time_text);
}


MyAccount::~MyAccount()
{
    delete ui;
}

//Display Status
void MyAccount::on_pushButton_clicked()
{
    Login conn;
    QString status;
    //username = ui->lineEdit_Username->text();
    status = ui->lineEdit_status->text();



    if(!conn.connOpen())
    {

         qDebug()<<"Failed to Open the Database";
         return;
    }

    conn.connOpen();
    QSqlQuery qry;
    //Use the status from the DB into the textbox
    qry.prepare("UPDATE login set Status='"+status+"' WHERE Name='"+username +"' ");

    if(status!=NULL)
    {

            if(qry.exec()) //if valid status, then save the updated status and display message
            {
                QMessageBox::critical(this,tr("Save"),tr("Changes saved!"));
                conn.connClose();
            }
            else
            {
                QMessageBox::critical(this,tr("Error"),qry.lastError().text());
            }
    }
    else //if status not entered and clicked update status
    {
          QMessageBox::critical(this,tr("Error"),tr("Enter a valid status"));
    }
}
//Sign out from the app
void MyAccount::on_button_signOut_clicked()
{
    //this->hide();
    this->close();
}

void MyAccount::on_toolButton_clicked()
{
    /*When the drop down button is clicked*/
}

void MyAccount::on_pushbutton_update_clicked()
{

}

//When Contact button is clicked, open the website in Default Browser
void MyAccount::on_pushButton_dev_clicked()
{
    QString link = "http://www.beta.shreyasg.com";
    QDesktopServices::openUrl(QUrl(link));
}


//Update user password into the Database
void MyAccount::on_pushButton_password_clicked()
{
    Login conn;
    QString password;
    //username = ui->lineEdit_Username->text();
    password = ui->lineEdit_password->text();


    if(!conn.connOpen())
    {

         qDebug()<<"Failed to Open the Database";
         return;
    }

    conn.connOpen();
    QSqlQuery qry;
    //qry.prepare("select into login (Password) values ('"+status+"')WHERE Name='"+username +"' ");
    qry.prepare("UPDATE login set Password='"+password+"' WHERE Name='"+username +"' ");

    /* Entered value in the field */
    if(password!=NULL)
    {

        if(qry.exec())
        {
            QMessageBox::information(this,tr("Save"),tr("Password successfully changed"));
            conn.connClose();
        }
        else //Show the exact error
        {
            QMessageBox::critical(this,tr("Error"),qry.lastError().text());
        }
    }
    /* For NULL password */
    else
    {
        QMessageBox::critical(this,tr("Error"),tr("Enter value in pasword"));
    }

}
