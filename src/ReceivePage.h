#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QProgressBar>
#include <QProcess>
#include <QFrame>
#include <QTimer>

class ReceivePage : public QWidget
{
    Q_OBJECT
public:
    explicit ReceivePage(QWidget *parent = nullptr);

private slots:
    void startReceive();
    void cancelReceive();
    void onProcessOutput();
    void onProcessFinished(int code);
    void openDownloadDir();
    void updateProgressAnimation();

private:
    void setupUi();
    void applyStyles();
    void setReceivingState(bool active);

    QLineEdit    *codeInput;
    QPushButton  *receiveBtn;
    QPushButton  *cancelBtn;
    QPushButton  *openDirBtn;
    QLabel       *statusLabel;
    QProgressBar *progressBar;
    QTextEdit    *logView;
    QFrame       *infoFrame;
    QLabel       *fileNameLabel;
    QLabel       *fileSizeLabel;
    QProcess     *process;
    QTimer       *progressTimer;
    int           progressValue;
};
