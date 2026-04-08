#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);

private slots:
    void saveSettings();
    void browseDownloadPath();

private:
    void setupUi();
    void applyStyles();
    void loadSettings();

    QLineEdit *relayEdit;
    QLineEdit *codeLengthEdit;
    QLineEdit *downloadPathEdit;
    QPushButton *saveBtn;
};

#endif // SETTINGSPAGE_H
