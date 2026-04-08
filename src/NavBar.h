#pragma once
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QStyle>

class NavBar : public QWidget
{
    Q_OBJECT
public:
    explicit NavBar(QWidget *parent = nullptr);
    void setActivePage(int index);

signals:
    void pageSelected(int index);

private:
    QList<QPushButton*> buttons;
    void makeButton(const QString &icon, const QString &label, int index, QHBoxLayout *layout);
};
