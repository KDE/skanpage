/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef SINGLE_OPTION_H
#define SINGLE_OPTION_H

#include <QObject>

namespace KSaneIface{
    class KSaneOption;
}

class SingleOption : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant name READ name NOTIFY optionReloaded)
    Q_PROPERTY(QVariant title READ title NOTIFY optionReloaded)
    Q_PROPERTY(QVariant description READ description NOTIFY optionReloaded)
    Q_PROPERTY(QVariant maximum READ maximum NOTIFY optionReloaded)
    Q_PROPERTY(QVariant minimum READ minimum NOTIFY optionReloaded)
    Q_PROPERTY(QVariant step READ step NOTIFY optionReloaded)
    Q_PROPERTY(QVariant valueList READ valueList NOTIFY optionReloaded)
    Q_PROPERTY(QVariant unit READ unit NOTIFY optionReloaded)
    Q_PROPERTY(QVariant type READ type NOTIFY optionReloaded)
    Q_PROPERTY(QVariant visible READ visible NOTIFY optionReloaded)
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)

public:
     explicit SingleOption(QObject *parent = nullptr);
    ~SingleOption();
    
    QVariant name() const;
    QVariant title() const;
    QVariant description() const;
    QVariant maximum() const;
    QVariant minimum() const;
    QVariant step() const;
    QVariant valueList() const;
    QVariant unit() const;
    QVariant type() const;
    QVariant visible() const;
    
    QVariant value() const;
    
    void clearOption();
    void setOption(KSaneIface::KSaneOption *option);

public Q_SLOTS:
    bool setValue(QVariant &value);

Q_SIGNALS:
    void optionReloaded();
    void valueChanged();
    
private:
    KSaneIface::KSaneOption *mOption = nullptr;
};

#endif // #define SINGLE_OPTION_H
