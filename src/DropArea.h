#ifndef DROPAREA_H
#define DROPAREA_H

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

class DropArea : public QFrame
{
    Q_OBJECT

public:
    explicit DropArea(QWidget *parent = nullptr) : QFrame(parent)
    {
        setAcceptDrops(true);
        setMinimumHeight(100);
        
        // Setup layout
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignCenter);
        
        QLabel *iconLabel = new QLabel(this);
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setText("📁");
        iconLabel->setStyleSheet("font-size: 48px;");
        
        QLabel *textLabel = new QLabel(this);
        textLabel->setAlignment(Qt::AlignCenter);
        textLabel->setText("Drag & drop files or folders here");
        textLabel->setStyleSheet("color: #9A90C0; font-size: 14px;");
        
        QLabel *hintLabel = new QLabel(this);
        hintLabel->setAlignment(Qt::AlignCenter);
        hintLabel->setText("or click buttons below to browse");
        hintLabel->setStyleSheet("color: #6A6480; font-size: 12px;");
        
        layout->addWidget(iconLabel);
        layout->addWidget(textLabel);
        layout->addWidget(hintLabel);
        
        setStyleSheet(R"(
            DropArea {
                background: #1C1A26;
                border: 2px dashed #3E2E70;
                border-radius: 12px;
            }
            DropArea:hover {
                border-color: #6252CC;
                background: #201D2E;
            }
        )");
    }

protected:
    void dragEnterEvent(QDragEnterEvent *event) override
    {
        if (event->mimeData()->hasUrls()) {
            event->acceptProposedAction();
            setStyleSheet(R"(
                DropArea {
                    background: #2E2648;
                    border: 2px solid #6252CC;
                    border-radius: 12px;
                }
            )");
        }
    }

    void dragLeaveEvent(QDragLeaveEvent *event) override
    {
        Q_UNUSED(event);
        setStyleSheet(R"(
            DropArea {
                background: #1C1A26;
                border: 2px dashed #3E2E70;
                border-radius: 12px;
            }
            DropArea:hover {
                border-color: #6252CC;
                background: #201D2E;
            }
        )");
    }

    void dropEvent(QDropEvent *event) override
    {
        setStyleSheet(R"(
            DropArea {
                background: #1C1A26;
                border: 2px dashed #3E2E70;
                border-radius: 12px;
            }
            DropArea:hover {
                border-color: #6252CC;
                background: #201D2E;
            }
        )");
        
        QStringList paths;
        const QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl &url : urls) {
            QString path = url.toLocalFile();
            if (!path.isEmpty()) {
                paths.append(path);
            }
        }
        
        if (!paths.isEmpty()) {
            emit filesDropped(paths);
        }
    }

signals:
    void filesDropped(const QStringList &paths);
};

#endif // DROPAREA_H
