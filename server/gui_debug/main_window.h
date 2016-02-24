#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "chat_client.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    user *me;
    char me_name[16];
    void update_lists(void);
    contact *current;

public:
    Q_SLOT void handle_packet(int);

    Q_SIGNAL void send_data(chat_message message);

private slots:
    void on_log_in_button_clicked();
    void on_create_account_clicked();
    void on_accept_button_clicked();
    void on_add_friend_button_clicked();
    void on_remove_button_clicked();
    void on_send_button_clicked();
    void comboSelection(int);
    void friend_chosen(int, int);
    void closeEvent(QCloseEvent *);

};

#endif // MAINWINDOW_H
