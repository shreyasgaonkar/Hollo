#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include "seconddialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    ui->label->setText("Hello!");
}

void MainWindow::on_pushButton_2_clicked()
{
    QMessageBox::information(this,tr("Title"),tr("This is a message"));
}

void MainWindow::on_pushButton_3_clicked()
{
    SecondDialog seconddialog;
    seconddialog.setModal(true);
    seconddialog.exec();
}
