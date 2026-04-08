#include <QScrollArea>
#include <QTimer>
#include "SettingsPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QSettings>
#include <QDir>

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent)
{
    setupUi();
    loadSettings();
    applyStyles();
}

void SettingsPage::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(32, 28, 32, 24);
    root->setSpacing(0);

    // Title
    auto *title = new QLabel("Settings");
    title->setObjectName("pageTitle");
    root->addWidget(title);
    root->addSpacing(20);

    // Scroll area for settings
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    auto *scrollWidget = new QWidget;
    auto *formLayout = new QFormLayout(scrollWidget);
    formLayout->setSpacing(16);
    formLayout->setLabelAlignment(Qt::AlignRight);

    // Relay URL
    relayEdit = new QLineEdit;
    relayEdit->setPlaceholderText("wss://relay.magic-wormhole.com");
    formLayout->addRow("Relay URL:", relayEdit);

    // Code length
    codeLengthEdit = new QLineEdit;
    codeLengthEdit->setPlaceholderText("3");
    codeLengthEdit->setText("3");
    formLayout->addRow("Code length:", codeLengthEdit);

    // Download path
    auto *downloadRow = new QHBoxLayout;
    downloadPathEdit = new QLineEdit;
    downloadPathEdit->setReadOnly(true);
    auto *browseBtn = new QPushButton("Browse...");
    connect(browseBtn, &QPushButton::clicked, this, &SettingsPage::browseDownloadPath);
    downloadRow->addWidget(downloadPathEdit, 1);
    downloadRow->addWidget(browseBtn);
    formLayout->addRow("Download path:", downloadRow);

    // Save button
    saveBtn = new QPushButton("Save settings");
    saveBtn->setObjectName("primaryBtn");
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &SettingsPage::saveSettings);

    scrollWidget->setLayout(formLayout);
    scroll->setWidget(scrollWidget);
    root->addWidget(scroll);
    root->addSpacing(20);
    root->addWidget(saveBtn);
    root->addStretch();
}

void SettingsPage::applyStyles()
{
    setStyleSheet(R"(
        SettingsPage { background: #12121A; }

        QLabel#pageTitle {
            font-size: 26px;
            font-weight: 700;
            color: #E8E6FE;
            letter-spacing: -0.3px;
        }

        QLabel {
            color: #C4BAFF;
            font-size: 13px;
            font-weight: 500;
        }

        QLineEdit {
            background: #1C1A26;
            border: 1px solid #2E2A40;
            border-radius: 8px;
            color: #E8E6FE;
            font-size: 13px;
            padding: 8px 12px;
        }

        QLineEdit:focus {
            border-color: #6252CC;
        }

        QPushButton {
            background: #2E2648;
            color: #BBAAFF;
            border: 1px solid #4A4060;
            border-radius: 6px;
            padding: 6px 14px;
            font-size: 12px;
        }

        QPushButton:hover {
            background: #3E3660;
        }

        QPushButton#primaryBtn {
            background: #6252CC;
            color: #fff;
            border: none;
            border-radius: 10px;
            padding: 12px 24px;
            font-size: 14px;
            font-weight: 600;
        }

        QPushButton#primaryBtn:hover {
            background: #7362DC;
        }
    )");
}

void SettingsPage::loadSettings()
{
    QSettings s;
    relayEdit->setText(s.value("relay", "").toString());
    codeLengthEdit->setText(s.value("codeLength", 3).toString());
    downloadPathEdit->setText(s.value("downloadPath",
                                      QDir::homePath() + "/Downloads").toString());
}

void SettingsPage::saveSettings()
{
    QSettings s;
    s.setValue("relay", relayEdit->text().trimmed());
    s.setValue("codeLength", codeLengthEdit->text().toInt());
    s.setValue("downloadPath", downloadPathEdit->text());

    saveBtn->setText("Saved!");
    QTimer::singleShot(1500, this, [this]{ saveBtn->setText("Save settings"); });
}

void SettingsPage::browseDownloadPath()
{
    QString path = QFileDialog::getExistingDirectory(this,
                                                     "Select Download Directory",
                                                     downloadPathEdit->text());
    if (!path.isEmpty()) {
        downloadPathEdit->setText(path);
    }
}
