#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QProgressBar>
#include <QListWidget>
#include <QProcess>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QTabBar>

class SendPage;
class ReceivePage;
class SettingsPage;
class NavBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void navigateTo(int index);

private:
    void setupUi();
    void applyStyleSheet();

    QWidget        *centralWidget;
    QStackedWidget *stack;
    NavBar         *navBar;
    SendPage       *sendPage;
    ReceivePage    *receivePage;
    SettingsPage   *settingsPage;
    QSystemTrayIcon *trayIcon;
};
