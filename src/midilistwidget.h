#ifndef MIDILISTWIDGET_H
#define MIDILISTWIDGET_H



#include <QWidget>
#include <QListWidget>
#include <QTableWidgetItem>
#include <QPushButton>



class MidiListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MidiListWidget(QWidget *parent = nullptr);

signals:
    void multiLinkButtonChecked(bool checked);
    void clearLinksButtonClicked();

public slots:
    void addPortName(QPair<QString, int> portId);
    void removePortName(QPair<QString, int> portId);
    void toggleMultiLinkButton(bool checked);

private:
    QListWidget* portsTable;
    QPushButton* multiLinkButton;

    QString getPortName(QPair<QString, int> portId);
};



#endif // MIDILISTWIDGET_H
