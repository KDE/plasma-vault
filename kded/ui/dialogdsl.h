/*
 *   Copyright 2017 by Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License or (at your option) version 3 or any later version
 *   accepted by the membership of KDE e.V. (or its successor approved
 *   by the membership of KDE e.V.), which shall act as a proxy
 *   defined in Section 14 of version 3 of the license.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PLASMAVAULT_KDED_UI_DIALOG_DSL_H
#define PLASMAVAULT_KDED_UI_DIALOG_DSL_H

#include <QString>
#include <QHash>
#include <QWidget>
#include <QVariant>

#include <functional>

#include <vault.h>

namespace DialogDsl {

// We want to have a normal ID for the QMap or QHash,
// but we also want it to have a translated name
class Key: public QByteArray {
public:
    Key(const QByteArray &id, const QString &translation = QString());

    QString translation() const;

    bool isExperimental() const;
    void setExperimental(bool experimental);

private:
    QString m_translation;
    bool m_isExperimental = false;

};

namespace operators {
    // A nicer way to define a Key and its translation
    inline
    Key operator/ (const char *id, const QString &name) {
        return Key(id, name);
    }

    inline
    Key experimental(Key key)
    {
        key.setExperimental(true);
        return key;
    }

}

// A dialog module can return a set of configured key-value pairs
class DialogModule: public QWidget {
    Q_OBJECT

    Q_PROPERTY(bool isValid READ isValid WRITE setIsValid NOTIFY isValidChanged)

public:
    DialogModule(bool isValid);

    virtual PlasmaVault::Vault::Payload fields() const = 0;
    virtual void init(const PlasmaVault::Vault::Payload &payload);

    virtual void aboutToBeShown();
    virtual bool shouldBeShown() const;
    virtual void aboutToBeHidden();

    bool isValid() const;
    void setIsValid(bool valid);


Q_SIGNALS:
    void isValidChanged(bool valid);

private:
    bool m_isValid;
};

typedef std::function<DialogModule*()> ModuleFactory;

typedef QVector<ModuleFactory> step;
typedef QVector<step> steps;
typedef QMap<Key, steps> Logic;

// If we want to have a single page containing multiple modules
class CompoundDialogModule: public DialogModule {
public:
    CompoundDialogModule(const step &children);

    PlasmaVault::Vault::Payload fields() const override;
    void init(const PlasmaVault::Vault::Payload &payload) override;

private:
    QVector<DialogModule*> m_children;

};

} // namespace DialogDsl

#endif // include guard

