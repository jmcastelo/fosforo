


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

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->addWidget(multiLinkButton);
    hLayout->addWidget(clearLinksButton);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout(hLayout);
    layout->addWidget(portsTable);

    setLayout(layout);

    connect(clearLinksButton, &QPushButton::clicked, this, &MidiListWidget::clearLinksButtonClicked);
    connect(multiLinkButton, &QPushButton::clicked, this, &MidiListWidget::multiLinkButtonChecked);
    connect(portsTable, &QListWidget::itemClicked, this, &MidiListWidget::portChecked);
}



void MidiListWidget::populatePortsTable(QList<QString> portNames)
{
    // portsTable->clear();

    // int portId = 0;


    int numItems = portsTable->count();

    for (int row = numItems - 1; row >= 0; row--)
    {
        QListWidgetItem* item = portsTable->item(row);

        if (item && !portNames.contains(item->text())) {
            delete item;
        }
    }

    foreach (QString portName, portNames)
    {
        /*QListWidgetItem* item = new QListWidgetItem(name, portsTable);
        QCheckBox* checkBox = new QCheckBox();
        portsTable->setItemWidget(item, checkBox);
        connect(checkBox, &QCheckBox::checkStateChanged, this, [=, this](Qt::CheckState state){
            emit portSelected(portId, state == Qt::Checked);
        });
        portId++;*/

        QList<QListWidgetItem*> items = portsTable->findItems(portName, Qt::MatchCaseSensitive);

        if (items.isEmpty())
        {
            QListWidgetItem* item = new QListWidgetItem(portName, portsTable);
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
        }

    }

}



void MidiListWidget::portChecked(QListWidgetItem* item)
{
    emit portSelected(item->text(), item->checkState() == Qt::Checked);
}



void MidiListWidget::checkPort(QString portName)
{
    QList<QListWidgetItem*> items = portsTable->findItems(portName, Qt::MatchCaseSensitive);
    if (!items.isEmpty()) {
        items[0]->setCheckState(Qt::Checked);
        emit portSelected(portName, true);
    }
}


void MidiListWidget::toggleMultiLinkButton(bool checked)
{
    multiLinkButton->setChecked(checked);
}
