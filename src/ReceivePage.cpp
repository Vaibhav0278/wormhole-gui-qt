#include <QRegularExpression>
#include "ReceivePage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QSettings>
#include <QTimer>

ReceivePage::ReceivePage(QWidget *parent) : QWidget(parent), process(nullptr), progressTimer(nullptr), progressValue(0)
{
    setupUi();
    applyStyles();

    // Create timer for continuous progress animation
    progressTimer = new QTimer(this);
    connect(progressTimer, &QTimer::timeout, this, &ReceivePage::updateProgressAnimation);
}

void ReceivePage::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(32, 28, 32, 24);
    root->setSpacing(0);

    // Title
    auto *title = new QLabel("Receive File");
    title->setObjectName("pageTitle");
    root->addWidget(title);
    root->addSpacing(8);

    auto *subtitle = new QLabel("Enter the wormhole code from the sender");
    subtitle->setObjectName("subtitleLabel");
    root->addWidget(subtitle);
    root->addSpacing(24);

    // Code input area
    auto *inputCard = new QFrame;
    inputCard->setObjectName("inputCard");
    auto *cardLayout = new QVBoxLayout(inputCard);
    cardLayout->setContentsMargins(20, 18, 20, 18);
    cardLayout->setSpacing(14);

    auto *inputRow = new QHBoxLayout;

    // Code icon
    auto *codeIcon = new QLabel("🔑");
    codeIcon->setFixedWidth(30);
    codeIcon->setStyleSheet("font-size: 20px; background: transparent;");

    codeInput = new QLineEdit;
    codeInput->setObjectName("codeInput");
    codeInput->setPlaceholderText("e.g.  7-crossover-clockwork");
    codeInput->setClearButtonEnabled(true);

    inputRow->addWidget(codeIcon);
    inputRow->addWidget(codeInput, 1);
    cardLayout->addLayout(inputRow);

    // Tip text
    auto *tipLabel = new QLabel(
        "The sender generates this code with  wormhole send <file>.\n"
        "You can also paste a code copied from the Wormhole Android app.");
    tipLabel->setObjectName("tipLabel");
    tipLabel->setWordWrap(true);
    cardLayout->addWidget(tipLabel);

    root->addWidget(inputCard);
    root->addSpacing(16);

    // Action buttons
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(10);

    receiveBtn = new QPushButton("  Receive file");
    receiveBtn->setObjectName("primaryBtn");
    receiveBtn->setCursor(Qt::PointingHandCursor);
    receiveBtn->setFixedHeight(48);
    connect(receiveBtn, &QPushButton::clicked, this, &ReceivePage::startReceive);
    connect(codeInput, &QLineEdit::returnPressed, this, &ReceivePage::startReceive);

    cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("dangerBtn");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setVisible(false);
    cancelBtn->setFixedHeight(48);
    connect(cancelBtn, &QPushButton::clicked, this, &ReceivePage::cancelReceive);

    btnRow->addWidget(receiveBtn, 3);
    btnRow->addWidget(cancelBtn, 1);
    root->addLayout(btnRow);
    root->addSpacing(18);

    // Info frame (shown during/after transfer)
    infoFrame = new QFrame;
    infoFrame->setObjectName("infoFrame");
    infoFrame->setVisible(false);

    auto *ifLayout = new QVBoxLayout(infoFrame);
    ifLayout->setContentsMargins(16, 14, 16, 14);
    ifLayout->setSpacing(8);

    auto *fileRow = new QHBoxLayout;
    auto *fileIcon = new QLabel("📥");
    fileIcon->setStyleSheet("font-size: 18px; background: transparent;");

    auto *fileInfo = new QVBoxLayout;
    fileNameLabel = new QLabel("Receiving...");
    fileNameLabel->setObjectName("fileNameLabel");
    fileSizeLabel = new QLabel("");
    fileSizeLabel->setObjectName("fileSizeLabel");
    fileInfo->addWidget(fileNameLabel);
    fileInfo->addWidget(fileSizeLabel);

    openDirBtn = new QPushButton("Open folder");
    openDirBtn->setObjectName("copyBtn");
    openDirBtn->setCursor(Qt::PointingHandCursor);
    openDirBtn->setVisible(false);
    connect(openDirBtn, &QPushButton::clicked, this, &ReceivePage::openDownloadDir);

    fileRow->addWidget(fileIcon);
    fileRow->addLayout(fileInfo, 1);
    fileRow->addWidget(openDirBtn);
    ifLayout->addLayout(fileRow);

    statusLabel = new QLabel("Connecting...");
    statusLabel->setObjectName("statusLabel");
    ifLayout->addWidget(statusLabel);

    progressBar = new QProgressBar;
    progressBar->setObjectName("transferProgress");
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(false);
    progressBar->setFixedHeight(5);
    ifLayout->addWidget(progressBar);

    root->addWidget(infoFrame);
    root->addSpacing(12);

    // Log
    auto *logTitle = new QLabel("Transfer log");
    logTitle->setObjectName("dimLabel");
    root->addWidget(logTitle);
    root->addSpacing(4);

    logView = new QTextEdit;
    logView->setObjectName("logView");
    logView->setReadOnly(true);
    logView->setMaximumHeight(140);
    logView->setPlaceholderText("Output will appear here...");
    root->addWidget(logView);

    root->addStretch();
}

void ReceivePage::applyStyles()
{
    setStyleSheet(R"(
        ReceivePage { background: #12121A; }

        QLabel#pageTitle {
            font-size: 26px;
            font-weight: 700;
            color: #E8E6FE;
        }
        QLabel#subtitleLabel {
            font-size: 13px;
            color: #7A7490;
        }
        QFrame#inputCard {
            background: #1C1A26;
            border: 1px solid #2E2A40;
            border-radius: 14px;
        }
        QLineEdit#codeInput {
            background: #12121A;
            border: 1.5px solid #3E3660;
            border-radius: 10px;
            color: #BBAAFF;
            font-size: 17px;
            font-family: monospace;
            font-weight: 600;
            letter-spacing: 1px;
            padding: 10px 14px;
        }
        QLineEdit#codeInput:focus { border-color: #6252CC; }
        QLineEdit#codeInput::placeholder { color: #4A4060; }

        QLabel#tipLabel {
            font-size: 12px;
            color: #6A6480;
            line-height: 1.5;
        }
        QPushButton#primaryBtn {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #6252CC, stop:1 #7362DC);
            color: white;
            border: none;
            border-radius: 24px;
            padding: 10px 24px;
            font-size: 14px;
            font-weight: 600;
        }
        QPushButton#primaryBtn:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #7362DC, stop:1 #8572EC);
        }
        QPushButton#primaryBtn:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #5242BC, stop:1 #6252CC);
        }
        QPushButton#primaryBtn:disabled {
            background: #2E2A40;
            color: #5A5470;
        }

        QPushButton#dangerBtn {
            background: #3A1A2A;
            color: #FF8080;
            border: 1px solid #6A2A3A;
            border-radius: 24px;
            padding: 10px 20px;
            font-size: 14px;
        }
        QPushButton#dangerBtn:hover {
            background: #4A2030;
            border-color: #8A3A4A;
        }
        QPushButton#dangerBtn:pressed {
            background: #2A1020;
        }

        QPushButton#copyBtn {
            background: #2E2648;
            color: #BBAAFF;
            border: 1px solid #4A4060;
            border-radius: 8px;
            padding: 6px 14px;
            font-size: 12px;
        }
        QPushButton#copyBtn:hover { background: #3E3660; }

        QFrame#infoFrame {
            background: #1C1A26;
            border: 1px solid #3E2E70;
            border-radius: 12px;
        }
        QLabel#fileNameLabel {
            font-size: 14px;
            font-weight: 600;
            color: #E8E6FE;
        }
        QLabel#fileSizeLabel {
            font-size: 12px;
            color: #7A7490;
        }
        QLabel#statusLabel {
            color: #9A90C0;
            font-size: 12px;
        }
        QLabel#dimLabel {
            color: #6A6480;
            font-size: 11px;
        }
        QProgressBar#transferProgress {
            background: #2A2640;
            border: none;
            border-radius: 3px;
        }
        QProgressBar#transferProgress::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #BBAAFF, stop:1 #D4C4FF);
            border-radius: 3px;
        }
        QTextEdit#logView {
            background: #0E0E16;
            color: #6A8060;
            border: 1px solid #1E1E2A;
            border-radius: 8px;
            font-family: monospace;
            font-size: 11px;
            padding: 6px;
        }
    )");
}

void ReceivePage::startReceive()
{
    QString code = codeInput->text().trimmed();
    if (code.isEmpty()) return;

    if (process) {
        process->kill();
        process->deleteLater();
    }

    QSettings s;
    QString relay = s.value("relay", "").toString();
    QString savePath = s.value("downloadPath",
                               QDir::homePath() + "/Downloads").toString();

                               QStringList args;
                               args << "receive";
                               if (!relay.isEmpty()) { args << "--relay-url" << relay; }
                               args << "--accept-file";
                               args << "--output-file" << savePath + "/";
                               args << code;

                               process = new QProcess(this);
                               process->setProcessChannelMode(QProcess::MergedChannels);
                               connect(process, &QProcess::readyRead, this, &ReceivePage::onProcessOutput);
                               connect(process, &QProcess::finished, this, &ReceivePage::onProcessFinished);

                               logView->clear();
                               fileNameLabel->setText("Connecting...");
                               fileSizeLabel->setText("");
                               statusLabel->setText("Connecting to relay...");
                               infoFrame->setVisible(true);
                               openDirBtn->setVisible(false);
                               setReceivingState(true);

                               process->start("wormhole", args);

                               if (!process->waitForStarted(2000)) {
                                   logView->append("[ERROR] Could not start 'wormhole'. Is magic-wormhole installed?\n"
                                   "Install with:  pip install magic-wormhole");
                                   setReceivingState(false);
                               }
}

void ReceivePage::cancelReceive()
{
    if (process) {
        process->kill();
        statusLabel->setText("Cancelled.");
        setReceivingState(false);
    }
}

void ReceivePage::onProcessOutput()
{
    QString out = process->readAll();
    logView->append(out.trimmed());

    // Parse filename
    QRegularExpression rxFile("Receiving file.*'(.+)'");
    QRegularExpressionMatch match = rxFile.match(out);
    if (match.hasMatch()) {
        fileNameLabel->setText(match.captured(1));
    }

    // Parse size
    QRegularExpression rxSize("(\\d+(?:\\.\\d+)?\\s*(?:B|KB|MB|GB))");
    match = rxSize.match(out);
    if (match.hasMatch()) {
        fileSizeLabel->setText(match.captured(1));
    }

    // Parse progress percentage
    QRegularExpression rxProgress("(\\d+)%");
    match = rxProgress.match(out);
    if (match.hasMatch()) {
        int percent = match.captured(1).toInt();
        progressBar->setValue(percent);
        statusLabel->setText(QString("Receiving... %1%").arg(percent));
        progressTimer->stop(); // Stop animation when we have real progress
    } else if (out.contains("Receiving") && !out.contains("%")) {
        statusLabel->setText("Receiving...");
        // Start continuous animation if no percentage available
        if (!progressTimer->isActive()) {
            progressTimer->start(30);
        }
    }

    if (out.contains("Received file")) {
        statusLabel->setText("✓ File received!");
        openDirBtn->setVisible(true);
        progressTimer->stop();
        progressBar->setValue(100);
    }
}

void ReceivePage::onProcessFinished(int code)
{
    setReceivingState(false);
    progressTimer->stop();
    if (code == 0) {
        progressBar->setValue(100);
        statusLabel->setText("✓ Transfer complete!");
    } else {
        statusLabel->setText("✗ Transfer failed");
        progressBar->setValue(0);
    }
}

void ReceivePage::updateProgressAnimation()
{
    // Continuous progress bar animation (indeterminate-style but with smooth movement)
    progressValue = (progressValue + 2) % 100;
    progressBar->setValue(progressValue);
}

void ReceivePage::openDownloadDir()
{
    QSettings s;
    QString path = s.value("downloadPath", QDir::homePath() + "/Downloads").toString();
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void ReceivePage::setReceivingState(bool active)
{
    receiveBtn->setEnabled(!active);
    codeInput->setEnabled(!active);
    cancelBtn->setVisible(active);

    if (active) {
        progressBar->setValue(0);
        progressValue = 0;
        progressTimer->start(30); // Start continuous animation
    } else {
        progressTimer->stop();
    }
}
