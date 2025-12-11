


#include "midilistwidget.h"

#include <QTableWidgetItem>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpinBox>



MidiListWidget::MidiListWidget(QWidget *parent): QWidget{parent}
{
    mMultiLinkButton = new QPushButton("Multi-assign");
    mMultiLinkButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mMultiLinkButton->setCheckable(true);

    QPushButton* clearLinksButton = new QPushButton("Clear assignments");
    clearLinksButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    mPortsTable = new QTableWidget;
    mPortsTable->setColumnCount(2);
    mPortsTable->setHorizontalHeaderLabels(QStringList { "Port Name", "Map" });
    mPortsTable->horizontalHeader()->setStretchLastSection(false);
    mPortsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    mPortsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mPortsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->addWidget(mMultiLinkButton);
    hLayout->addWidget(clearLinksButton);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout(hLayout);
    layout->addWidget(mPortsTable);

    setLayout(layout);

    connect(clearLinksButton, &QPushButton::clicked, this, &MidiListWidget::clearLinksButtonClicked);
    connect(mMultiLinkButton, &QPushButton::clicked, this, &MidiListWidget::multiLinkButtonChecked);
}



void MidiListWidget::addPortEntry(MidiInputPort* inPort)
{
    const int row = mPortsTable->rowCount();
    mPortsTable->insertRow(row);

    QString portName = inPort->portName();
    QTableWidgetItem* nameItem = new QTableWidgetItem(portName);
    mPortsTable->setItem(row, 0, nameItem);

    QSpinBox* mapSpinBox = new QSpinBox;
    mapSpinBox->setRange(1, 16);
    mapSpinBox->setValue(inPort->map() + 1);
    mapSpinBox->setAlignment(Qt::AlignCenter);

    connect(mapSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [inPort](int value) {
        inPort->setMap(value - 1);
    });

    mPortsTable->setCellWidget(row, 1, mapSpinBox);

    mMidiInPorts.append(inPort);
}



void MidiListWidget::removePortEntry(MidiInputPort* inPort)
{
    int row = mMidiInPorts.lastIndexOf(inPort);
    mPortsTable->removeRow(row);
    mMidiInPorts.removeOne(inPort);
}



void MidiListWidget::toggleMultiLinkButton(bool checked)
{
    mMultiLinkButton->setChecked(checked);
}
