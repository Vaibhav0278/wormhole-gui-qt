#include "MainWindow.h"
#include "SendPage.h"
#include "ReceivePage.h"
#include "SettingsPage.h"
#include "NavBar.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Wormhole");
    setMinimumSize(820, 600);
    resize(960, 680);

    // Center on screen
    auto *screen = QApplication::primaryScreen();
    QRect sg = screen->availableGeometry();
    move(sg.center() - rect().center());

    setupUi();
    applyStyleSheet();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    stack = new QStackedWidget;
    sendPage     = new SendPage;
    receivePage  = new ReceivePage;
    settingsPage = new SettingsPage;

    stack->addWidget(sendPage);
    stack->addWidget(receivePage);
    stack->addWidget(settingsPage);

    navBar = new NavBar;

    mainLayout->addWidget(stack, 1);
    mainLayout->addWidget(navBar);

    connect(navBar, &NavBar::pageSelected, this, &MainWindow::navigateTo);
    navigateTo(0);
}

void MainWindow::navigateTo(int index)
{
    stack->setCurrentIndex(index);
    navBar->setActivePage(index);
}

void MainWindow::applyStyleSheet()
{
    setStyleSheet(R"(
        QMainWindow, QWidget#centralWidget {
            background-color: #12121A;
        }
        /* Scrollbars */
        QScrollBar:vertical {
            background: #1C1A26;
            width: 6px;
            border-radius: 3px;
        }
        QScrollBar::handle:vertical {
            background: #3D3650;
            border-radius: 3px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar:horizontal { height: 0; }

        /* Tooltips */
        QToolTip {
            background: #2A2740;
            color: #E8E6FE;
            border: 1px solid #4A4464;
            border-radius: 6px;
            padding: 4px 8px;
            font-size: 12px;
        }
    )");
}
