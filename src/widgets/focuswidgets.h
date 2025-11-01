/*
*  Copyright 2025 Jose Maria Castelo Ares
*
*  Contact: <jose.maria.castelo@gmail.com>
*  Repository: <https://github.com/jmcastelo/fosforo>
*
*  This file is part of Fosforo.
*
*  Fosforo is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  Fosforo is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with Fosforo.  If not, see <https://www.gnu.org/licenses/>.
*/



#ifndef FOCUSWIDGETS_H
#define FOCUSWIDGETS_H



#include <QLineEdit>
#include <QSlider>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>



// A custom QLineEdit that signals focus out and in

class FocusLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    FocusLineEdit(QWidget* parent = nullptr) : QLineEdit(parent)
    {
        setFixedWidth(80);
    }

signals:
    void focusOut();
    void focusIn();

protected:
    void focusOutEvent(QFocusEvent* event)
    {
        QLineEdit::focusOutEvent(event);
        emit focusOut();
    }
    void focusInEvent(QFocusEvent* event)
    {
        QLineEdit::focusInEvent(event);
        emit focusIn();
    }
};


// A custom QSlider that signals focus out and in

class FocusSlider : public QSlider
{
    Q_OBJECT

public:
    FocusSlider(Qt::Orientation orientation, QWidget* parent = nullptr) : QSlider(orientation, parent) {}

protected:
    void focusOutEvent(QFocusEvent* event)
    {
        QSlider::focusOutEvent(event);
        emit focusOut();
    }
    void focusInEvent(QFocusEvent* event)
    {
        QSlider::focusInEvent(event);
        emit focusIn();
    }

signals:
    void focusOut();
    void focusIn();
};

// A custom QComboBox that signals focus out and in

class FocusComboBox : public QComboBox
{
    Q_OBJECT

public:
    FocusComboBox(QWidget* parent = nullptr) : QComboBox(parent) {}

protected:
    void focusOutEvent(QFocusEvent* event)
    {
        QComboBox::focusOutEvent(event);
        emit focusOut();
    }
    void focusInEvent(QFocusEvent* event)
    {
        QComboBox::focusInEvent(event);
        emit focusIn();
    }

signals:
    void focusOut();
    void focusIn();
};

// A custom QPushButton that signals focus out and in

class FocusPushButton : public QPushButton
{
    Q_OBJECT

public:
    FocusPushButton(QWidget* parent = nullptr) : QPushButton(parent) {}

protected:
    void focusOutEvent(QFocusEvent* event)
    {
        QPushButton::focusOutEvent(event);
        emit focusOut();
    }
    void focusInEvent(QFocusEvent* event)
    {
        QPushButton::focusInEvent(event);
        emit focusIn();
    }

signals:
    void focusOut();
    void focusIn();
};



// A custom QPushButton that signals focus out and in

class FocusGroupBox : public QGroupBox
{
    Q_OBJECT

public:
    FocusGroupBox(QWidget* parent = nullptr) : QGroupBox(parent) {}

protected:
    void focusOutEvent(QFocusEvent* event)
    {
        QGroupBox::focusOutEvent(event);
        emit focusOut();
    }
    void focusInEvent(QFocusEvent* event)
    {
        QGroupBox::focusInEvent(event);
        emit focusIn();
    }

signals:
    void focusOut();
    void focusIn();
};



#endif // FOCUSWIDGETS_H
