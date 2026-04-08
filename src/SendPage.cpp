#include <QRegularExpression>
#include "SendPage.h"
#include "DropArea.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QApplication>
#include <QClipboard>
#include <QFileInfo>
#include <QMimeData>
#include <QSettings>
#include <QTimer>
#include <QPainter>
#include <qrencode.h>

SendPage::SendPage(QWidget *parent) : QWidget(parent), process(nullptr), pulsePhase(0), qrExpanded(false), currentProgress(0)
{
    setupUi();
    applyStyles();

    pulseTimer = new QTimer(this);
    connect(pulseTimer, &QTimer::timeout, this, [this]{
        pulsePhase = (pulsePhase + 1) % 6;
        QString dots = QString(".").repeated(pulsePhase % 4);
        statusLabel->setText("Waiting for receiver" + dots);
    });
}

void SendPage::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(32, 28, 32, 24);
    root->setSpacing(0);

    // Title
    auto *title = new QLabel("Send File");
    title->setObjectName("pageTitle");
    root->addWidget(title);
    root->addSpacing(20);

    // Drop area
    dropArea = new DropArea;
    dropArea->setMinimumHeight(140);
    connect(dropArea, &DropArea::filesDropped, this, &SendPage::addItems);
    root->addWidget(dropArea);
    root->addSpacing(14);

    // File list
    fileList = new QListWidget;
    fileList->setObjectName("fileList");
    fileList->setMaximumHeight(160);
    fileList->setAlternatingRowColors(true);
    fileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    fileList->setContextMenuPolicy(Qt::ActionsContextMenu);
    auto *removeAction = new QAction("Remove selected", fileList);
    connect(removeAction, &QAction::triggered, this, [this]{
        qDeleteAll(fileList->selectedItems());
        selectedPaths.clear();
        for (int i = 0; i < fileList->count(); ++i)
            selectedPaths << fileList->item(i)->data(Qt::UserRole).toString();
    });
    fileList->addAction(removeAction);
    root->addWidget(fileList);
    root->addSpacing(14);

    // Buttons row - pill shaped buttons
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(12);

    sendFileBtn = new QPushButton("📄  Send file");
    sendFileBtn->setObjectName("pillBtn");
    sendFileBtn->setCursor(Qt::PointingHandCursor);
    sendFileBtn->setFixedHeight(40);
    connect(sendFileBtn, &QPushButton::clicked, this, &SendPage::pickFiles);

    sendFolderBtn = new QPushButton("📁  Send folder");
    sendFolderBtn->setObjectName("pillBtn");
    sendFolderBtn->setCursor(Qt::PointingHandCursor);
    sendFolderBtn->setFixedHeight(40);
    connect(sendFolderBtn, &QPushButton::clicked, this, &SendPage::pickFolder);

    btnRow->addWidget(sendFileBtn);
    btnRow->addWidget(sendFolderBtn);
    root->addLayout(btnRow);
    root->addSpacing(14);

    // Send / Cancel buttons
    auto *actionRow = new QHBoxLayout;
    actionRow->setSpacing(10);

    sendBtn = new QPushButton("🚀  Generate wormhole code");
    sendBtn->setObjectName("primaryPillBtn");
    sendBtn->setCursor(Qt::PointingHandCursor);
    sendBtn->setFixedHeight(48);
    connect(sendBtn, &QPushButton::clicked, this, &SendPage::startSend);

    cancelBtn = new QPushButton("✖  Cancel");
    cancelBtn->setObjectName("dangerPillBtn");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setVisible(false);
    cancelBtn->setFixedHeight(48);
    connect(cancelBtn, &QPushButton::clicked, this, &SendPage::cancelSend);

    actionRow->addWidget(sendBtn, 3);
    actionRow->addWidget(cancelBtn, 1);
    root->addLayout(actionRow);
    root->addSpacing(16);

    // Result frame with QR code
    resultFrame = new QFrame;
    resultFrame->setObjectName("resultFrame");
    resultFrame->setVisible(false);

    auto *rfLayout = new QVBoxLayout(resultFrame);
    rfLayout->setContentsMargins(16, 14, 16, 14);
    rfLayout->setSpacing(10);

    // Code row
    auto *codeRow = new QHBoxLayout;
    auto *codeTitleLbl = new QLabel("Wormhole Code");
    codeTitleLbl->setObjectName("dimLabel");

    codeLabel = new QLabel("—");
    codeLabel->setObjectName("codeLabel");
    codeLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    copyBtn = new QPushButton("📋  Copy");
    copyBtn->setObjectName("copyPillBtn");
    copyBtn->setCursor(Qt::PointingHandCursor);
    copyBtn->setFixedSize(80, 32);
    connect(copyBtn, &QPushButton::clicked, this, &SendPage::copyCode);

    codeRow->addWidget(codeTitleLbl);
    codeRow->addWidget(codeLabel, 1);
    codeRow->addWidget(copyBtn);
    rfLayout->addLayout(codeRow);

    // QR Code section
    auto *qrHeaderRow = new QHBoxLayout;
    auto *qrTitleLbl = new QLabel("QR Code (Click to expand)");
    qrTitleLbl->setObjectName("dimLabel");

    qrHeaderRow->addWidget(qrTitleLbl);
    qrHeaderRow->addStretch();
    rfLayout->addLayout(qrHeaderRow);

    // QR Code label - clickable
    qrCodeLabel = new QLabel;
    qrCodeLabel->setObjectName("qrCodeLabel");
    qrCodeLabel->setAlignment(Qt::AlignCenter);
    qrCodeLabel->setMinimumSize(150, 150);
    qrCodeLabel->setMaximumSize(200, 200);
    qrCodeLabel->setScaledContents(true);
    qrCodeLabel->setCursor(Qt::PointingHandCursor);
    qrCodeLabel->installEventFilter(this);
    rfLayout->addWidget(qrCodeLabel, 0, Qt::AlignCenter);

    statusLabel = new QLabel("Waiting for receiver...");
    statusLabel->setObjectName("statusLabel");
    rfLayout->addWidget(statusLabel);

    // Continuous progress bar
    progressBar = new QProgressBar;
    progressBar->setObjectName("transferProgress");
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(true);
    progressBar->setFormat("%p%");
    progressBar->setFixedHeight(20);
    rfLayout->addWidget(progressBar);

    root->addWidget(resultFrame);

    // Transfer log area
    auto *logTitle = new QLabel("Transfer Log");
    logTitle->setObjectName("dimLabel");
    root->addSpacing(4);
    root->addWidget(logTitle);
    root->addSpacing(4);

    logView = new QTextEdit;
    logView->setObjectName("logView");
    logView->setReadOnly(true);
    logView->setMaximumHeight(110);
    logView->setPlaceholderText("Transfer progress will appear here...");
    root->addWidget(logView);

    root->addStretch();
}

void SendPage::applyStyles()
{
    setStyleSheet(R"(
        SendPage { background: #12121A; }

        QLabel#pageTitle {
            font-size: 26px;
            font-weight: 700;
            color: #E8E6FE;
        }

        QListWidget#fileList {
            background: #1C1A26;
            border: 1px solid #2E2A40;
            border-radius: 10px;
            color: #C4BAFF;
            font-size: 13px;
            padding: 4px;
        }
        QListWidget#fileList::item {
            padding: 6px 10px;
            border-radius: 6px;
        }
        QListWidget#fileList::item:selected {
            background: #2E2648;
            color: #BBAAFF;
        }

        /* All pill-shaped buttons */
        QPushButton#pillBtn {
            background: #2E2648;
            color: #BBAAFF;
            border: 1.5px solid #5242BC;
            border-radius: 20px;
            padding: 8px 20px;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton#pillBtn:hover {
            background: #3E3660;
            border-color: #7362DC;
        }
        QPushButton#pillBtn:pressed {
            background: #1E1638;
        }

        QPushButton#primaryPillBtn {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #6252CC, stop:1 #7362DC);
            color: white;
            border: none;
            border-radius: 24px;
            padding: 10px 24px;
            font-size: 14px;
            font-weight: 600;
        }
        QPushButton#primaryPillBtn:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #7362DC, stop:1 #8572EC);
        }
        QPushButton#primaryPillBtn:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #5242BC, stop:1 #6252CC);
        }
        QPushButton#primaryPillBtn:disabled {
            background: #2E2A40;
            color: #5A5470;
        }

        QPushButton#dangerPillBtn {
            background: #3A1A2A;
            color: #FF8080;
            border: 1px solid #6A2A3A;
            border-radius: 24px;
            padding: 10px 20px;
            font-size: 14px;
        }
        QPushButton#dangerPillBtn:hover {
            background: #4A2030;
            border-color: #8A3A4A;
        }
        QPushButton#dangerPillBtn:pressed {
            background: #2A1020;
        }

        QPushButton#copyPillBtn {
            background: #2E2648;
            color: #BBAAFF;
            border: 1px solid #4A4060;
            border-radius: 16px;
            padding: 4px 12px;
            font-size: 12px;
        }
        QPushButton#copyPillBtn:hover {
            background: #3E3660;
        }
        QPushButton#copyPillBtn:pressed {
            background: #1E1638;
        }

        QFrame#resultFrame {
            background: #1C1A26;
            border: 1px solid #3E2E70;
            border-radius: 12px;
        }

        QLabel#codeLabel {
            font-size: 17px;
            font-weight: 700;
            color: #BBAAFF;
            font-family: monospace;
        }

        QLabel#qrCodeLabel {
            background: white;
            border: 2px solid #3E2E70;
            border-radius: 12px;
            padding: 10px;
        }
        QLabel#qrCodeLabel:hover {
            border-color: #6252CC;
            background: #f0f0ff;
        }

        QLabel#statusLabel {
            color: #9A90C0;
            font-size: 12px;
        }

        QLabel#dimLabel {
            color: #6A6480;
            font-size: 11px;
            text-transform: uppercase;
        }

        QProgressBar#transferProgress {
            background: #2A2640;
            border: none;
            border-radius: 10px;
            text-align: center;
        }
        QProgressBar#transferProgress::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #6252CC, stop:1 #BBAAFF);
            border-radius: 10px;
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

void SendPage::generateQRCode(const QString &code)
{
    if (code.isEmpty()) {
        qrCodeLabel->setText("📱\nNo code yet");
        qrCodeLabel->setStyleSheet("font-size: 24px; color: #6A6480; background: #1C1A26;");
        return;
    }

    // Generate real QR code using libqrencode
    QRcode *qr = QRcode_encodeString(code.toUtf8().constData(), 0, QR_ECLEVEL_L, QR_MODE_8, 1);

    if (qr) {
        int size = qrExpanded ? 400 : 200;
        QPixmap pixmap(size, size);
        pixmap.fill(Qt::white);

        QPainter painter(&pixmap);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);

        int cellSize = size / qr->width;
        for (int y = 0; y < qr->width; y++) {
            for (int x = 0; x < qr->width; x++) {
                if (qr->data[y * qr->width + x] & 1) {
                    painter.drawRect(x * cellSize, y * cellSize, cellSize, cellSize);
                }
            }
        }

        painter.end();
        QRcode_free(qr);

        qrCodeLabel->setPixmap(pixmap);
    } else {
        qrCodeLabel->setText("QR generation failed");
    }
}

bool SendPage::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == qrCodeLabel && event->type() == QEvent::MouseButtonPress) {
        toggleQRExpand();
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void SendPage::toggleQRExpand()
{
    // Create a dialog for expanded QR code
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Wormhole QR Code");
    dialog->setModal(false);
    dialog->setMinimumSize(500, 550);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    QLabel *titleLabel = new QLabel("Scan this QR code with your phone");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #E8E6FE;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    QLabel *codeDisplay = new QLabel(currentCode);
    codeDisplay->setStyleSheet("font-size: 14px; color: #BBAAFF; font-family: monospace;");
    codeDisplay->setAlignment(Qt::AlignCenter);
    layout->addWidget(codeDisplay);

    QLabel *bigQr = new QLabel;
    bigQr->setAlignment(Qt::AlignCenter);
    bigQr->setMinimumSize(400, 400);

    // Generate larger QR code
    QRcode *qr = QRcode_encodeString(currentCode.toUtf8().constData(), 0, QR_ECLEVEL_L, QR_MODE_8, 1);
    if (qr) {
        QPixmap pixmap(400, 400);
        pixmap.fill(Qt::white);

        QPainter painter(&pixmap);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);

        int cellSize = 400 / qr->width;
        for (int y = 0; y < qr->width; y++) {
            for (int x = 0; x < qr->width; x++) {
                if (qr->data[y * qr->width + x] & 1) {
                    painter.drawRect(x * cellSize, y * cellSize, cellSize, cellSize);
                }
            }
        }

        painter.end();
        QRcode_free(qr);
        bigQr->setPixmap(pixmap);
    }

    layout->addWidget(bigQr);

    QPushButton *closeBtn = new QPushButton("Close");
    closeBtn->setStyleSheet("QPushButton { background: #6252CC; color: white; border: none; border-radius: 25px; padding: 10px 20px; font-size: 14px; } QPushButton:hover { background: #7362DC; }");
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setFixedHeight(40);
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    layout->addWidget(closeBtn);

    dialog->setStyleSheet("QDialog { background: #12121A; }");
    dialog->exec();
}

void SendPage::addItems(const QStringList &paths)
{
    for (const QString &path : paths) {
        if (!selectedPaths.contains(path)) {
            selectedPaths << path;
            QFileInfo fi(path);
            auto *item = new QListWidgetItem;
            item->setText((fi.isDir() ? "📁 " : "📄 ") + fi.fileName());
            item->setData(Qt::UserRole, path);
            fileList->addItem(item);
        }
    }
}

void SendPage::pickFiles()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Select files to send",
        QString(),
                                                      "All files (*.*)"
    );

    if (!files.isEmpty()) {
        addItems(files);
    }
}

void SendPage::pickFolder()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Select folder to send",
        QString(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!dir.isEmpty()) {
        addItems({dir});
    }
}

void SendPage::startSend()
{
    if (selectedPaths.isEmpty()) {
        statusLabel->setText("Please select files or a folder first.");
        resultFrame->setVisible(true);
        return;
    }

    if (process) { process->kill(); process->deleteLater(); }

    QSettings s;
    QString relay   = s.value("relay", "").toString();
    int codeLen     = s.value("codeLength", 3).toInt();

    QStringList args;
    args << "send";
    if (!relay.isEmpty()) { args << "--relay-url" << relay; }
    args << "--code-length" << QString::number(codeLen);
    for (const QString &p : selectedPaths) args << p;

    process = new QProcess(this);
    process->setProcessChannelMode(QProcess::MergedChannels);
    connect(process, &QProcess::readyRead, this, &SendPage::onProcessOutput);
    connect(process, &QProcess::finished, this, &SendPage::onProcessFinished);

    logView->clear();
    currentCode.clear();
    codeLabel->setText("Generating...");
    statusLabel->setText("Starting...");
    progressBar->setValue(0);
    resultFrame->setVisible(true);
    setTransferState(true);

    process->start("wormhole", args);

    if (!process->waitForStarted(2000)) {
        logView->append("[ERROR] Could not start 'wormhole'. Is magic-wormhole installed?");
        setTransferState(false);
    }
}

void SendPage::cancelSend()
{
    if (process) {
        process->kill();
        statusLabel->setText("Cancelled.");
        setTransferState(false);
    }
}

void SendPage::onProcessOutput()
{
    QString out = process->readAll();
    logView->append(out.trimmed());

    // Extract percentage for continuous progress
    QRegularExpression percentRx("(\\d+)%");
    QRegularExpressionMatch match = percentRx.match(out);
    if (match.hasMatch()) {
        int percent = match.captured(1).toInt();
        currentProgress = percent;
        progressBar->setValue(percent);

        if (percent == 100) {
            statusLabel->setText("✓ Transfer complete!");
        } else if (percent > 0) {
            statusLabel->setText(QString("Transferring... %1%").arg(percent));
        }
    }

    QString code = extractCode(out);
    if (!code.isEmpty() && currentCode.isEmpty()) {
        currentCode = code;
        codeLabel->setText(code);
        generateQRCode(code);
        statusLabel->setText("Waiting for receiver...");
        pulseTimer->start(600);
    }

    if (out.contains("Sending") && currentProgress == 0) {
        statusLabel->setText("Starting transfer...");
        progressBar->setRange(0, 100);
    }
}

void SendPage::onProcessFinished(int code)
{
    pulseTimer->stop();
    setTransferState(false);
    if (code == 0 && currentProgress < 100) {
        progressBar->setValue(100);
        statusLabel->setText("✓ Transfer complete!");
    } else if (code != 0) {
        statusLabel->setText("Transfer ended (code " + QString::number(code) + ")");
    }
}

void SendPage::copyCode()
{
    if (!currentCode.isEmpty()) {
        QApplication::clipboard()->setText(currentCode);
        copyBtn->setText("✓ Copied!");
        QTimer::singleShot(1500, this, [this]{ copyBtn->setText("📋  Copy"); });
    }
}

void SendPage::setTransferState(bool active)
{
    sendBtn->setEnabled(!active);
    sendFileBtn->setEnabled(!active);
    sendFolderBtn->setEnabled(!active);
    cancelBtn->setVisible(active);
    if (!active && currentProgress == 100) {
        progressBar->setValue(100);
    } else if (!active) {
        progressBar->setValue(0);
    }
}

QString SendPage::extractCode(const QString &output)
{
    QRegularExpression rx("code is: (\\S+)");
    QRegularExpressionMatch match = rx.match(output);
    if (match.hasMatch()) return match.captured(1);

    QRegularExpression rx2("(\\d+-[a-z]+-[a-z]+(?:-[a-z]+)*)");
    match = rx2.match(output);
    if (match.hasMatch()) return match.captured(1);

    return QString();
}
