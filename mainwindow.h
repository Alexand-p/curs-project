#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_addBookButton_clicked();
    void on_editBookButton_clicked();
    void on_deleteBookButton_clicked();
    void on_addUserButton_clicked();
    void on_editUserButton_clicked();
    void on_deleteUserButton_clicked();
    void on_issueBookButton_clicked();
    void on_returnBookButton_clicked();
    void on_generateReportButton_clicked();
    void on_loginButton_clicked();
    void onLoginSuccess(const QString& username, const QString& userType);

private:
    Ui::MainWindow *ui;
    QStandardItemModel *bookModel;
    QStandardItemModel *userModel;

    QLineEdit *usernameLineEdit;
    QLineEdit *passwordLineEdit;
    QPushButton *loginButton;
    QLabel *loginStatusLabel;

    int loggedInUserIndex;

    void loadBooks();
    void loadUsers();
    void saveBooks();
    void saveUsers();
    void updateBookView();
    void updateUserView();
    void addBook(const QString &title, const QString &author, const QString &genre, int yearPublished, bool available);
    void editBook(int row, const QString &title, const QString &author, const QString &genre, int yearPublished, bool available);
    void deleteBook(int row);
    void addUser(const QString &username, const QString &password, const QString &userType);
    void editUser(int row, const QString &username, const QString &password, const QString &userType);
    void deleteUser(int row);
    void issueBook(int userRow, int bookRow);
    void returnBook(int userRow, int bookRow);
    void generateReport();

    bool validateLogin(const QString& username, const QString& password, QString& userType); // Обновлен метод
    void setElementsEnabled(bool enabled);
};

#endif
