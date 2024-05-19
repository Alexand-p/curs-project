#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QMessageBox>
#include <QJsonArray>
#include <QGridLayout>
#include <QStandardItemModel>
#include <QStandardItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , loggedInUserIndex(-1) // Инициализация флага
{
    ui->setupUi(this);
    bookModel = new QStandardItemModel(this);
    userModel = new QStandardItemModel(this);
    ui->bookTableView->setModel(bookModel);
    ui->userTableView->setModel(userModel);

    usernameLineEdit = new QLineEdit;
    passwordLineEdit = new QLineEdit;
    passwordLineEdit->setEchoMode(QLineEdit::Password);
    loginButton = new QPushButton("Войти");
    loginStatusLabel = new QLabel("");
    loginStatusLabel->setStyleSheet("color: green;");

    QGridLayout *loginLayout = new QGridLayout;
    loginLayout->addWidget(new QLabel("Имя пользователя:"), 0, 0);
    loginLayout->addWidget(usernameLineEdit, 0, 1);
    loginLayout->addWidget(new QLabel("Пароль:"), 1, 0);
    loginLayout->addWidget(passwordLineEdit, 1, 1);
    loginLayout->addWidget(loginButton, 2, 0, 1, 2);
    loginLayout->addWidget(loginStatusLabel, 3, 0, 1, 2);

    QWidget *loginWidget = new QWidget;
    loginWidget->setLayout(loginLayout);
    ui->verticalLayout->addWidget(loginWidget);

    setElementsEnabled(false);
    loadBooks();
    loadUsers();
    ui->userTableView->setVisible(false);
    ui->bookTableView->setVisible(false);

    connect(loginButton, &QPushButton::clicked, this, &MainWindow::on_loginButton_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_addBookButton_clicked()
{
    bool ok;
    QString title = QInputDialog::getText(this, "Добавить книгу", "Название:", QLineEdit::Normal, "", &ok);
    if (ok && !title.isEmpty()) {
        QString author = QInputDialog::getText(this, "Добавить книгу", "Автор:", QLineEdit::Normal, "", &ok);
        if (ok && !author.isEmpty()) {
            QString genre = QInputDialog::getText(this, "Добавить книгу", "Жанр:", QLineEdit::Normal, "", &ok);
            if (ok && !genre.isEmpty()) {
                int yearPublished = QInputDialog::getInt(this, "Добавить книгу", "Год издания:", 2000, 1900, 2024, 1, &ok);
                if (ok) {
                    int userRow = ui->userTableView->currentIndex().row();
                    if (userRow >= 0) {
                        if (userModel->item(userRow, 2)->text() == "Администратор") {
                            qDebug() << "Adding book: " << title << author << genre << yearPublished;
                            addBook(title, author, genre, yearPublished, true);
                            saveBooks();
                            loadBooks();
                        } else {
                            QMessageBox::warning(this, "Ошибка доступа", "У вас недостаточно прав для добавления книг.");
                        }
                    }
                }
            }
        }
    }
}

void MainWindow::on_editBookButton_clicked()
{
    int row = ui->bookTableView->currentIndex().row();
    if (row >= 0) {
        QString title = bookModel->item(row, 0)->text();
        QString author = bookModel->item(row, 1)->text();
        QString genre = bookModel->item(row, 2)->text();
        int yearPublished = bookModel->item(row, 3)->text().toInt();
        bool available = bookModel->item(row, 4)->text() == "Доступна";
        bool ok;
        QString newTitle = QInputDialog::getText(this, "Редактировать книгу", "Название:", QLineEdit::Normal, title, &ok);
        if (ok && !newTitle.isEmpty()) {
            QString newAuthor = QInputDialog::getText(this, "Редактировать книгу", "Автор:", QLineEdit::Normal, author, &ok);
            if (ok && !newAuthor.isEmpty()) {
                QString newGenre = QInputDialog::getText(this, "Редактировать книгу", "Жанр:", QLineEdit::Normal, genre, &ok);
                if (ok && !newGenre.isEmpty()) {
                    int newYearPublished = QInputDialog::getInt(this, "Редактировать книгу", "Год издания:", yearPublished, 1900, 2024, 1, &ok);
                    if (ok) {
                        editBook(row, newTitle, newAuthor, newGenre, newYearPublished, available);
                        saveBooks();
                        loadBooks();
                    }
                }
            }
        }
    }
}

void MainWindow::on_deleteBookButton_clicked()
{
    int row = ui->bookTableView->currentIndex().row();
    if (row >= 0) {
        if (QMessageBox::question(this, "Удаление книги", "Вы уверены, что хотите удалить эту книгу?") == QMessageBox::Yes) {
            deleteBook(row);
            saveBooks();
            loadBooks();
        }
    }
}

void MainWindow::on_addUserButton_clicked()
{
    bool ok;
    QString username = QInputDialog::getText(this, "Добавить пользователя", "Имя пользователя:", QLineEdit::Normal, "", &ok);
    if (ok && !username.isEmpty()) {
        QString password = QInputDialog::getText(this, "Добавить пользователя", "Пароль:", QLineEdit::Password, "", &ok);
        if (ok && !password.isEmpty()) {
            QString userType = QInputDialog::getItem(this, "Добавить пользователя", "Тип пользователя:", QStringList() << "Администратор" << "Пользователь", 0, false, &ok);
            if (ok) {
                addUser(username, password, userType);
                saveUsers();
                loadUsers();
            }
        }
    }
}

void MainWindow::on_editUserButton_clicked()
{
    int row = ui->userTableView->currentIndex().row();
    if (row >= 0) {
        QString username = userModel->item(row, 0)->text();
        QString password = userModel->item(row, 1)->text();
        QString userType = userModel->item(row, 2)->text();
        bool ok;
        QString newUsername = QInputDialog::getText(this, "Редактировать пользователя", "Имя пользователя:", QLineEdit::Normal, username, &ok);
        if (ok && !newUsername.isEmpty()) {
            QString newPassword = QInputDialog::getText(this, "Редактировать пользователя", "Пароль:", QLineEdit::Password, password, &ok);
            if (ok && !newPassword.isEmpty()) {
                QString newUserType = QInputDialog::getItem(this, "Редактировать пользователя", "Тип пользователя:", QStringList() << "Администратор" << "Пользователь", 0, false, &ok);
                if (ok) {
                    editUser(row, newUsername, newPassword, newUserType);
                    saveUsers();
                    loadUsers();
                }
            }
        }
    }
}

void MainWindow::on_deleteUserButton_clicked()
{
    int row = ui->userTableView->currentIndex().row();
    if (row >= 0) {
        if (QMessageBox::question(this, "Удаление пользователя", "Вы уверены, что хотите удалить этого пользователя?") == QMessageBox::Yes) {
            deleteUser(row);
            saveUsers();
            loadUsers();
        }
    }
}

void MainWindow::on_issueBookButton_clicked()
{
    if (loggedInUserIndex >= 0 && userModel->item(loggedInUserIndex, 2)->text() == "Пользователь") {
        int bookRow = ui->bookTableView->currentIndex().row();
        if (bookRow >= 0) {
            if (bookModel->item(bookRow, 4)->text() == "Доступна") {
                issueBook(loggedInUserIndex, bookRow);
                saveBooks();
                loadBooks();
            } else {
                QMessageBox::warning(this, "Ошибка", "Книга не доступна для взятия.");
            }
        }
    } else {
        int userRow = ui->userTableView->currentIndex().row();
        if (userRow >= 0) {
            int bookRow = ui->bookTableView->currentIndex().row();
            if (bookRow >= 0) {
                if (bookModel->item(bookRow, 4)->text() == "Доступна") {
                    issueBook(userRow, bookRow);
                    saveBooks();
                    loadBooks();
                } else {
                    QMessageBox::warning(this, "Ошибка", "Книга не доступна для выдачи.");
                }
            }
        }
    }
}

void MainWindow::on_returnBookButton_clicked()
{
    if (loggedInUserIndex >= 0 && userModel->item(loggedInUserIndex, 2)->text() == "Пользователь") {
        QList<int> userBooks;
        for (int i = 0; i < bookModel->rowCount(); ++i) {
            if (bookModel->item(i, 5)->text() == userModel->item(loggedInUserIndex, 0)->text()) {
                userBooks.append(i);
            }
        }
        if (userBooks.isEmpty()) {
            QMessageBox::information(this, "Возврат книги", "У вас нет книг для возврата.");
            return;
        }
        bool ok;
        QStringList bookTitles;
        for (int i : userBooks) {
            bookTitles.append(bookModel->item(i, 0)->text());
        }
        QString bookTitle = QInputDialog::getItem(this, "Возврат книги", "Выберите книгу для возврата:", bookTitles, 0, false, &ok);
        if (ok && !bookTitle.isEmpty()) {
            for (int i : userBooks) {
                if (bookModel->item(i, 0)->text() == bookTitle) {
                    returnBook(loggedInUserIndex, i);
                    saveBooks();
                    loadBooks();
                    break;
                }
            }
        }
    } else {
        int userRow = ui->userTableView->currentIndex().row();
        if (userRow >= 0) {
            int bookRow = ui->bookTableView->currentIndex().row();
            if (bookRow >= 0) {
                if (bookModel->item(bookRow, 4)->text() == "Взята") {
                    returnBook(userRow, bookRow);
                    saveBooks();
                    loadBooks();
                } else {
                    QMessageBox::warning(this, "Ошибка", "Книга не взята пользователем.");
                }
            }
        }
    }
}

void MainWindow::on_generateReportButton_clicked()
{
    generateReport();
}

void MainWindow::loadBooks()
{
    QFile file("books.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Could not open books.json for reading.");
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    if (!jsonDoc.isArray()) {
        qWarning("Invalid JSON format in books.json.");
        file.close();
        return;
    }

    QJsonArray jsonArray = jsonDoc.array();
    bookModel->clear();
    bookModel->setHorizontalHeaderLabels(QStringList() << "Название" << "Автор" << "Жанр" << "Год издания" << "Статус" << "Взял");
    for (int i = 0; i < jsonArray.size(); i++) {
        QJsonObject bookObject = jsonArray.at(i).toObject();
        QString title = bookObject.value("title").toString();
        QString author = bookObject.value("author").toString();
        QString genre = bookObject.value("genre").toString();
        int yearPublished = bookObject.value("yearPublished").toInt();
        bool available = bookObject.value("available").toBool();
        QString takenByUser = bookObject.value("takenByUser").toString();
        QList<QStandardItem*> items;
        items.append(new QStandardItem(title));
        items.append(new QStandardItem(author));
        items.append(new QStandardItem(genre));
        items.append(new QStandardItem(QString::number(yearPublished)));
        items.append(new QStandardItem(available ? "Доступна" : "Взята"));
        items.append(new QStandardItem(takenByUser));
        bookModel->appendRow(items);
    }

    file.close();
    qDebug() << "Books loaded from books.json, total:" << jsonArray.size();
}

void MainWindow::loadUsers()
{
    QFile file("users.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Could not open users.json for reading.");
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    if (!jsonDoc.isArray()) {
        qWarning("Invalid JSON format in users.json.");
        file.close();
        return;
    }

    QJsonArray jsonArray = jsonDoc.array();
    userModel->clear();
    userModel->setHorizontalHeaderLabels(QStringList() << "Имя пользователя" << "Пароль" << "Тип пользователя");
    for (int i = 0; i < jsonArray.size(); i++) {
        QJsonObject userObject = jsonArray.at(i).toObject();
        QString username = userObject.value("username").toString();
        QString password = userObject.value("password").toString();
        QString userType = userObject.value("userType").toString();
        QList<QStandardItem*> items;
        items.append(new QStandardItem(username));
        items.append(new QStandardItem(password));
        items.append(new QStandardItem(userType));
        userModel->appendRow(items);
    }

    file.close();
    qDebug() << "Users loaded from users.json, total:" << jsonArray.size();
}

void MainWindow::saveBooks()
{
    qDebug() << "Calling saveBooks...";

    QFile file("books.json");
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Could not open books.json for writing.");
        return;
    }

    QJsonArray jsonArray;
    for (int i = 0; i < bookModel->rowCount(); i++) {
        QJsonObject bookObject;
        bookObject.insert("title", bookModel->item(i, 0)->text());
        bookObject.insert("author", bookModel->item(i, 1)->text());
        bookObject.insert("genre", bookModel->item(i, 2)->text());
        bookObject.insert("yearPublished", bookModel->item(i, 3)->text().toInt());
        bookObject.insert("available", bookModel->item(i, 4)->text() == "Доступна");
        bookObject.insert("takenByUser", bookModel->item(i, 5)->text());
        jsonArray.append(bookObject);
    }

    QJsonDocument jsonDoc(jsonArray);
    file.write(jsonDoc.toJson());
    file.close();

    // Отладочная информация
    qDebug() << "Saved books to books.json, total:" << bookModel->rowCount();
}

void MainWindow::saveUsers()
{
    QFile file("users.json");
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Could not open users.json for writing.");
        return;
    }

    QJsonArray jsonArray;
    for (int i = 0; i < userModel->rowCount(); i++) {
        QJsonObject userObject;
        userObject.insert("username", userModel->item(i, 0)->text());
        userObject.insert("password", userModel->item(i, 1)->text());
        userObject.insert("userType", userModel->item(i, 2)->text());
        jsonArray.append(userObject);
    }

    QJsonDocument jsonDoc(jsonArray);
    file.write(jsonDoc.toJson());
    file.close();

    qDebug() << "Saved users to users.json, total:" << userModel->rowCount();
}

void MainWindow::updateBookView()
{
    ui->bookTableView->resizeColumnsToContents();
    ui->bookTableView->resizeRowsToContents();
}

void MainWindow::updateUserView()
{
    ui->userTableView->resizeColumnsToContents();
    ui->userTableView->resizeRowsToContents();
}

void MainWindow::addBook(const QString &title, const QString &author, const QString &genre, int yearPublished, bool available)
{
    QList<QStandardItem*> items;
    items.append(new QStandardItem(title));
    items.append(new QStandardItem(author));
    items.append(new QStandardItem(genre));
    items.append(new QStandardItem(QString::number(yearPublished)));
    items.append(new QStandardItem(available ? "Доступна" : "Взята"));
    items.append(new QStandardItem(""));
    bookModel->appendRow(items);

    // Отладочная информация
    qDebug() << "Added book: " << title << author << genre << yearPublished << available;

    saveBooks();  // Сохранение данных сразу после добавления книги
}

void MainWindow::editBook(int row, const QString &title, const QString &author, const QString &genre, int yearPublished, bool available)
{
    bookModel->setItem(row, 0, new QStandardItem(title));
    bookModel->setItem(row, 1, new QStandardItem(author));
    bookModel->setItem(row, 2, new QStandardItem(genre));
    bookModel->setItem(row, 3, new QStandardItem(QString::number(yearPublished)));
    bookModel->setItem(row, 4, new QStandardItem(available ? "Доступна" : "Взята"));
}

void MainWindow::deleteBook(int row)
{
    bookModel->removeRow(row);
    saveBooks();  // Сохранение данных сразу после удаления книги
}

void MainWindow::addUser(const QString &username, const QString &password, const QString &userType)
{
    QList<QStandardItem*> items;
    items.append(new QStandardItem(username));
    items.append(new QStandardItem(password));
    items.append(new QStandardItem(userType));
    userModel->appendRow(items);
}

void MainWindow::editUser(int row, const QString &username, const QString &password, const QString &userType)
{
    userModel->setItem(row, 0, new QStandardItem(username));
    userModel->setItem(row, 1, new QStandardItem(password));
    userModel->setItem(row, 2, new QStandardItem(userType));
}

void MainWindow::deleteUser(int row)
{
    userModel->removeRow(row);
}

void MainWindow::issueBook(int userRow, int bookRow)
{
    QString username = userModel->item(userRow, 0)->text();
    bookModel->setItem(bookRow, 4, new QStandardItem("Взята"));
    bookModel->setItem(bookRow, 5, new QStandardItem(username));
    saveBooks();  // Сохранение данных после выдачи книги
}

void MainWindow::returnBook(int userRow, int bookRow)
{
    bookModel->setItem(bookRow, 4, new QStandardItem("Доступна"));
    bookModel->setItem(bookRow, 5, new QStandardItem(""));
    saveBooks();  // Сохранение данных после возврата книги
}

void MainWindow::generateReport()
{
    QMessageBox::information(this, "Отчет", "Отчет пока не реализован!");
}

void MainWindow::onLoginSuccess(const QString& username, const QString& userType)
{
    loadBooks();
    loadUsers();

    if (userType == "Пользователь") {
        // Скрыть все ненужные кнопки и изменить кнопку на "Взять книгу"
        ui->addBookButton->setVisible(false);
        ui->editBookButton->setVisible(false);
        ui->deleteBookButton->setVisible(false);
        ui->addUserButton->setVisible(false);
        ui->editUserButton->setVisible(false);
        ui->deleteUserButton->setVisible(false);
        ui->generateReportButton->setVisible(false);
        ui->issueBookButton->setText("Взять книгу");

        for (int i = 0; i < userModel->rowCount(); ++i) {
            if (userModel->item(i, 0)->text() == username) {
                loggedInUserIndex = i;
                ui->userTableView->setRowHidden(i, false);
            } else {
                ui->userTableView->setRowHidden(i, true);
            }
        }
    } else {
        ui->userTableView->setVisible(true);
    }

    ui->bookTableView->setVisible(true);
    show();
}

void MainWindow::on_loginButton_clicked()
{
    QString username = usernameLineEdit->text();
    QString password = passwordLineEdit->text();

    QString userType;
    if (validateLogin(username, password, userType)) {
        loginStatusLabel->setText("Вход успешен!");
        setElementsEnabled(true);
        onLoginSuccess(username, userType);
    } else {
        loginStatusLabel->setText("Неверные данные. Попробуйте еще раз.");
    }
}

bool MainWindow::validateLogin(const QString& username, const QString& password, QString& userType)
{
    QFile file("users.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Could not open users.json for reading.");
        return false;
    }

    QByteArray data = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    if (!jsonDoc.isArray()) {
        qWarning("Invalid JSON format in users.json.");
        file.close();
        return false;
    }

    QJsonArray jsonArray = jsonDoc.array();
    for (int i = 0; i < jsonArray.size(); i++) {
        QJsonObject userObject = jsonArray.at(i).toObject();
        QString storedUsername = userObject.value("username").toString();
        QString storedPassword = userObject.value("password").toString();
        QString storedUserType = userObject.value("userType").toString();

        if (storedUsername == username && storedPassword == password) {
            userType = storedUserType;
            file.close();
            return true;
        }
    }

    file.close();
    return false;
}

void MainWindow::setElementsEnabled(bool enabled)
{
    ui->addBookButton->setEnabled(enabled);
    ui->editBookButton->setEnabled(enabled);
    ui->deleteBookButton->setEnabled(enabled);
    ui->addUserButton->setEnabled(enabled);
    ui->editUserButton->setEnabled(enabled);
    ui->deleteUserButton->setEnabled(enabled);
    ui->issueBookButton->setEnabled(enabled);
    ui->returnBookButton->setEnabled(enabled);
    ui->generateReportButton->setEnabled(enabled);
    ui->bookTableView->setEnabled(enabled);
    ui->userTableView->setEnabled(enabled);
}
