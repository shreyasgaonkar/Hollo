#include "login.h"
#include "ui_login.h"
Login::Login(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);

    //to show image in the Login GUI
    QPixmap pix("/media/shreyas/DATA/Photography/Shortlisted/bean.jpg");
    ui->label_image->setPixmap(pix);

    //to check if connection to the Database is established or not
    if(!connOpen())
        ui->label->setText("Failed to open the Database");
    else
        ui->label->setText("Connected to Database");



}

Login::~Login()
{
    delete ui;
}

QString username, password, status, userEmail;
/*Put it ouside the function to make it accessible by other function*/

void Login::on_pushButton_clicked()
{
    //Use the value from the textbox and store into variables for later use
    username = ui->lineEdit_Username->text();
    password = ui->lineEdit_Password->text();


    if(!connOpen())
    {

         qDebug()<<"Failed to Open the Database";
         return;
    }
    //Verify the user's password from the Database named login and open connection
    connOpen();
    QSqlQuery qry; //defined to initialized SQL Query in QT Creator
    qry.prepare("SELECT * from login WHERE Name='"+username +"' and Password='"+password+"'");


    if(qry.exec())
    {
        int count=0;
        while(qry.next())
        {count++;}

            if(count==1){
        ui->label->setText("Username and password is correct");

        connClose();
        this->hide();
        MyAccount myaccount;
        myaccount.setModal(true);
        myaccount.exec();

            }

            if(count>1)
        ui->label->setText("Duplicate Username and password");

            if(count<1)
        ui->label->setText("INCORRECT Username and password!");

        }

    //use the user's status update in my account
    connOpen();
    QSqlQuery qry1;
    qry1.prepare("SELECT Status from login WHERE Name='"+username);


}



