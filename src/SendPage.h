// SendPage.h
#ifndef SEND_PAGE_H
#define SEND_PAGE_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QFrame>
#include <QTextEdit>
#include <QProgressBar>
#include <QProcess>
#include <QTimer>

class DropArea;

class SendPage : public QWidget
{
    Q_OBJECT

public:
    explicit SendPage(QWidget *parent = nullptr);

private slots:
    void pickFiles();
    void pickFolder();
    void startSend();
    void cancelSend();
    void onProcessOutput();
    void onProcessFinished(int code);
    void copyCode();
    void toggleQRExpand();
    void addItems(const QStringList &paths);

private:
    // UI Components
    DropArea *dropArea;
    QListWidget *fileList;
    QPushButton *sendFileBtn;
    QPushButton *sendFolderBtn;
    QPushButton *sendBtn;
    QPushButton *cancelBtn;
    QFrame *resultFrame;
    QLabel *codeLabel;
    QPushButton *copyBtn;
    QLabel *qrCodeLabel;
    QLabel *statusLabel;
    QProgressBar *progressBar;
    QTextEdit *logView;

    // State
    QStringList selectedPaths;
    QProcess *process;
    QTimer *pulseTimer;
    int pulsePhase;
    bool qrExpanded;
    QString currentCode;
    int currentProgress;

    // Methods
    void setupUi();
    void applyStyles();
    void generateQRCode(const QString &code);
    void setTransferState(bool active);
    QString extractCode(const QString &output);
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // SEND_PAGE_H
