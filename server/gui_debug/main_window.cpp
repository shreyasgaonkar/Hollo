#include "main_window.h"
#include "ui_main_window.h"

extern chat_client *clientp;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    current = NULL;

    ui->stackedWidget->setCurrentWidget(ui->logInPage);

    QObject::connect(clientp, SIGNAL(received_packet(int)), this, SLOT(handle_packet(int)));
    QObject::connect(this, SIGNAL(send_data(chat_message)), clientp, SLOT(send_packet(chat_message)));


    ui->status_combo->addItem("Available");
    ui->status_combo->addItem("Busy");
    ui->status_combo->addItem("Offline");
    ui->status_combo->addItem("Away");

    ui->status_combo->setCurrentIndex(3);
    connect(ui->status_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(comboSelection(int)));

    ui->friends_list->setColumnCount(2);
    //TODO ui->friends_list->setHorizontalHeader("Status" << "Name");
    ui->pending_list->setColumnCount(1);

    QObject::connect(ui->friends_list, SIGNAL(cellClicked(int,int)), this, SLOT(friend_chosen(int,int)));


    std::cout << "Hello there\n";
}



MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::friend_chosen(int r, int c) {
    current = me->friends[r];
    std::cout << "R: " << r << " C: " << c << " name: " << current->name << std::endl;
    ui->user_label->setText(current->name.c_str());
    update_lists();
}

void MainWindow::update_lists(void) {
    ui->friends_list->setRowCount(me->friends.size());
    for (int i = 0; i < me->friends.size(); ++i) {
        ui->friends_list->setItem(i, 0, new QTableWidgetItem(std::to_string(me->friends[i]->status).c_str()));
        ui->friends_list->setItem(i, 1, new QTableWidgetItem(me->friends[i]->name.c_str()));
    }
    ui->pending_list->setRowCount(me->pending.size());
    for (int i = 0; i < me->pending.size(); ++i) {
        std::cout << "name: " << me->pending[i]->name << "\n";
        ui->pending_list->setItem(i, 0, new QTableWidgetItem(me->pending[i]->name.c_str()));
    }
    ui->chat->clear();
    if (current != NULL) {
        for (int i = 0; i < current->c_messages.size(); ++i) {
            std::string msg = "";
            msg += std::to_string(current->c_messages[i].get_from()) + " ";
            msg += std::string(current->c_messages[i].body(), current->c_messages[i].body_length());
            msg += "\n";
            ui->chat->append(msg.c_str());
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    std::cout << "User is exiting the application!\n";
    if (me) {
        //create a log out message and send it out
        chat_message log_out;
        log_out.body_length(0);
        log_out.set_data(LOG_M, L_OUT, 0,0);
        log_out.encode_header();

        emit send_data(log_out);
    }
    sleep(2);
    event->accept();
}

void MainWindow::handle_packet(int x) {
    std::cout << "I'm in main window printing out a message!\n";

    chat_message m = clientp->get_message();
    m.print_message();

    char fname[16]; std::string fsname;
    int accept_f;

    switch (m.get_type()) {
    case (TEXT_M):
        std::cout << "Message from: " << m.get_from()
        << ": " << m.body() << std::endl;
        //TODO add to the right friend
        for (int i = 0; i < me->friends.size(); ++i) {
            if (me->friends[i]->id == m.get_from()) {
                me->friends[i]->c_messages.push_back(m);
            }
        }
        update_lists();
    break;
    case (LOG_M):
    if (m.get_code_num() == L_SUCCESS) {
          std::cout << "Log in successful!\n";
          me = new user(me_name, "no_matter", m.get_to());

          ui->stackedWidget->setCurrentWidget(ui->chat_page);

          //ask for pending friends and statuses
          /*
          chat_message ask_pending;
          ask_pending.set_data(FRIEND, F_GET_PENDING, m.get_to(), 0);
          ask_pending.body_length(0);
          ask_pending.encode_header();

          emit send_data(ask_pending);
          */
          chat_message pending_s;
          pending_s.set_data(STATUS, S_GET_STATUSES, m.get_to(), 0);
          pending_s.body_length(0);
          pending_s.encode_header();

          emit send_data(pending_s);

    }
    break;
    case (CREAT_ACC):
    if (m.get_code_num() == C_ACC_SUCCESS) {
          std::cout << "Account cretaed successfully";
          ui->log_label->setText("Account created Successfully!");
    }
    break;
    case (STATUS): {
          //find the friend and change the status
            //if I already have this person in my friends than just update the status,
            //otherwise add the person to friends
        int from = m.get_from();
        for (int i = 0; i < me->friends.size(); ++i) {
            if (me->friends[i]->id == from) {
                me->friends[i]->status = m.get_code_num() - S_CHANGE_MIN;
                update_lists();
                return;
            }
        }
        char s_name[16] = {0};
        std::memcpy(s_name, m.body(), 16);
        //didn't find him in my friend's list. Let's add a new friend
        me->friends.push_back(new contact(m.get_from(), std::string(s_name), m.get_code_num() - S_CHANGE_MIN));
        update_lists();
    }
    break;
    case (FRIEND):
    switch (m.get_code_num()) {
        case (F_REQUEST):
            char f_name[16] = {0};
            std::memcpy(f_name, m.body(), 16);
            me->pending.push_back(new contact(m.get_from(),f_name,user::OFFLINE));
            std::cout << "Pushed: " << f_name << "\n";
            update_lists();
        break;
       // case (F_REMOVE):

        //break;
    }
    break;
    case (ERR):
    std::cout << "Error: " << m.body() << std::endl;
    std::string error = std::string(m.body());
    error.resize(m.body_length());
    ui->log_label->setText(error.c_str());
    break;
    }
}

void MainWindow::on_log_in_button_clicked() {
    std::cout << "Clicked log in button\n";

    chat_message message;
    message.body_length(32); //always 32 bytes!

    char user_name_[16] = {0};
    char password_[16] = {0};

    std::memcpy(me_name, user_name_, 16);
    std::memcpy(user_name_, ui->user_name->text().toStdString().c_str(), ui->user_name->text().toStdString().length());
    std::memcpy(password_, ui->password->text().toStdString().c_str(), ui->password->text().toStdString().length());

    std::memcpy(message.body(), user_name_, 16);
    std::memcpy(message.body()+16, password_, 16);

    message.set_data(LOG_M, L_IN, 0, 0);

    message.encode_header();

    emit send_data(message);
}

void MainWindow::on_create_account_clicked() {
    std::cout << "Clicked create account button\n";
    chat_message message;
    message.body_length(32); //always 32 bytes!

    char user_name_[16] = {0};
    char password_[16] = {0};

    std::memcpy(user_name_, ui->user_name->text().toStdString().c_str(), ui->user_name->text().toStdString().length());
    std::memcpy(password_, ui->password->text().toStdString().c_str(), ui->password->text().toStdString().length());

    std::memcpy(message.body(), user_name_, 16);
    std::memcpy(message.body()+16, password_, 16);

    message.set_data(CREAT_ACC, C_ACC_SUCCESS, 0, 0);

    message.encode_header();

    emit send_data(message);
}

void MainWindow::on_accept_button_clicked() {
    QModelIndexList il = ui->pending_list->selectionModel()->selection().indexes();
    QModelIndex itemi = il.at(0);
    int row = itemi.row();
    //me->friends.push_back(me->pending[row]);


    chat_message accept_m;
    accept_m.body_length(0);
    accept_m.set_data(FRIEND, F_ACCEPT, me->my_info.id, me->pending[row]->id);
    accept_m.encode_header();
    me->pending.erase(me->pending.begin() + row);
    update_lists();

    emit send_data(accept_m);
}

void MainWindow::on_add_friend_button_clicked() {
    char friend_name[16] = {0};
    int input_length;
    std::string fname = ui->input_line->text().toStdString();
    input_length = fname.length() < 16 ? fname.length() : 16;

    std::memcpy(friend_name, fname.c_str(), input_length);

    chat_message message;
    message.body_length(16);
    std::memcpy(message.body(), friend_name, 16);
    message.set_data(FRIEND, F_REQUEST, me->my_info.id, 0);
    message.encode_header();

    emit send_data(message);
    ui->input_line->clear();
}

void MainWindow::on_remove_button_clicked() {

}

void MainWindow::on_send_button_clicked() {
    if (current == NULL) return;
    chat_message text_m;
    text_m.body_length(ui->input_line->text().toStdString().length());
    std::memcpy(text_m.body(), ui->input_line->text().toStdString().c_str(),
                ui->input_line->text().toStdString().length());
    ui->input_line->clear();
    text_m.set_data(TEXT_M, TEXT_C, me->my_info.id, current->id);
    text_m.encode_header();
    emit send_data(text_m);

    current->c_messages.push_back(text_m);
    update_lists();
}

void MainWindow::comboSelection(int selection) {

    chat_message message;
    std::memcpy(message.body(), me_name, 16);
    message.body_length(16);

    message.set_data(STATUS, (selection + 1) + S_CHANGE_MIN, me->my_info.id, 0);

    message.encode_header();

    emit send_data(message);
}
