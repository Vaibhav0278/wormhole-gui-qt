#include <QApplication>
#include <QStyleFactory>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Wormhole");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("WormholeQt");
    app.setStyle(QStyleFactory::create("Fusion"));

    // Dark palette
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window,          QColor(18, 18, 22));
    darkPalette.setColor(QPalette::WindowText,      QColor(232, 230, 254));
    darkPalette.setColor(QPalette::Base,            QColor(28, 26, 38));
    darkPalette.setColor(QPalette::AlternateBase,   QColor(38, 36, 50));
    darkPalette.setColor(QPalette::ToolTipBase,     QColor(18, 18, 22));
    darkPalette.setColor(QPalette::ToolTipText,     QColor(232, 230, 254));
    darkPalette.setColor(QPalette::Text,            QColor(232, 230, 254));
    darkPalette.setColor(QPalette::Button,          QColor(38, 36, 50));
    darkPalette.setColor(QPalette::ButtonText,      QColor(232, 230, 254));
    darkPalette.setColor(QPalette::BrightText,      Qt::white);
    darkPalette.setColor(QPalette::Link,            QColor(187, 170, 255));
    darkPalette.setColor(QPalette::Highlight,       QColor(98, 82, 204));
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text,       QColor(120, 114, 150));
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(120, 114, 150));
    app.setPalette(darkPalette);

    MainWindow w;
    w.show();
    return app.exec();
}
