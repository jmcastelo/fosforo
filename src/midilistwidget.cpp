


#include "midilistwidget.h"

#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QVBoxLayout>



MidiListWidget::MidiListWidget(QWidget *parent): QWidget{parent}
{
    multiLinkButton = new QPushButton("Multi-assign");
    multiLinkButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    multiLinkButton->setCheckable(true);

    QPushButton* clearLinksButton = new QPushButton("Clear assignments");
    clearLinksButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    portsTable = new QListWidget;
    portsTable->setSelectionMode(QAbstractItemView::NoSelection);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->addWidget(multiLinkButton);
    hLayout->addWidget(clearLinksButton);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout(hLayout);
    layout->addWidget(portsTable);

    setLayout(layout);

    connect(clearLinksButton, &QPushButton::clicked, this, &MidiListWidget::clearLinksButtonClicked);
    connect(multiLinkButton, &QPushButton::clicked, this, &MidiListWidget::multiLinkButtonChecked);
}



void MidiListWidget::addPortName(QPair<QString, int> portId)
{
    QString portName = getPortName(portId);
    QListWidgetItem* item = new QListWidgetItem(portName, portsTable);
    item->setFlags(Qt::ItemIsEnabled);

    portsTable->sortItems();
}



void MidiListWidget::removePortName(QPair<QString, int> portId)
{
    QString portName = getPortName(portId);

    auto items = portsTable->findItems(portName, Qt::MatchCaseSensitive);

    if (!items.isEmpty()) {
        portsTable->removeItemWidget(items[0]);
        delete items[0];
    }

    portsTable->sortItems();
}



void MidiListWidget::toggleMultiLinkButton(bool checked)
{
    multiLinkButton->setChecked(checked);
}



QString MidiListWidget::getPortName(QPair<QString, int> portId)
{
    return portId.first + " [" + QString::number(portId.second + 1) + "]";
}
