#include "NavBar.h"
#include <QLabel>
#include <QVBoxLayout>

NavBar::NavBar(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(70);
    setObjectName("NavBar");

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    makeButton("↑", "Send",     0, layout);
    makeButton("↓", "Receive",  1, layout);
    makeButton("⚙", "Settings", 2, layout);

    setStyleSheet(R"(
        #NavBar {
            background-color: #1C1A26;
            border-top: 1px solid #2E2A40;
        }
        QPushButton {
            background: transparent;
            border: none;
            color: #7A7490;
            font-size: 11px;
            padding: 8px 16px 6px;
        }
        QPushButton:hover { color: #C4BAFF; }
        QPushButton[active="true"] {
            color: #BBAAFF;
        }
    )");
}

void NavBar::makeButton(const QString &icon, const QString &label, int index, QHBoxLayout *layout)
{
    auto *btn = new QPushButton(this);
    btn->setObjectName(QString("navBtn%1").arg(index));

    // Layout: icon on top, label below
    auto *vbox = new QVBoxLayout(btn);
    vbox->setContentsMargins(0, 4, 0, 4);
    vbox->setSpacing(2);

    auto *iconLbl = new QLabel(icon);
    iconLbl->setAlignment(Qt::AlignCenter);
    iconLbl->setStyleSheet("font-size: 24px; background: transparent; border: none;");  // Increased from 18px to 24px

    auto *textLbl = new QLabel(label);
    textLbl->setAlignment(Qt::AlignCenter);
    textLbl->setStyleSheet("font-size: 11px; background: transparent; border: none;");

    vbox->addWidget(iconLbl);
    vbox->addWidget(textLbl);

    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect(btn, &QPushButton::clicked, this, [this, index]{ emit pageSelected(index); });
    layout->addWidget(btn);
    buttons.append(btn);
}

void NavBar::setActivePage(int index)
{
    for (int i = 0; i < buttons.size(); ++i) {
        buttons[i]->setProperty("active", i == index);
        buttons[i]->style()->unpolish(buttons[i]);
        buttons[i]->style()->polish(buttons[i]);

        // Tint the active button's background pill
        if (i == index)
            buttons[i]->setStyleSheet(
                "background: #2E2648; border-radius: 20px; color: #BBAAFF; "
                "font-size: 11px; padding: 6px 20px;");
            else
                buttons[i]->setStyleSheet(
                    "background: transparent; border: none; color: #7A7490; "
                    "font-size: 11px; padding: 6px 20px;");
    }
}
