/*
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

#include <KConfigGroup>
#include <KSharedConfig>

namespace PlasmaQuick
{
class SharedQmlEngine;
}

class QTimer;
class ShellCorona;

class Osd : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.osdService")
public:
    Osd(const KSharedConfig::Ptr &config, ShellCorona *corona);
    ~Osd() override;

public Q_SLOTS:
    void capsLockEnabledChanged(bool capsLockEnabled);
    void numLockEnabledChanged(bool numLockEnabled);
    void showText(const QString &icon, const QString &text);

Q_SIGNALS:
    void osdProgress(const QString &icon, const int percent, const int maximumPercent, const QString &additionalText);
    void osdText(const QString &icon, const QString &text);

private Q_SLOTS:
    void hideOsd();

private:
    bool init();
    void showOsd();

    ShellCorona *const m_corona;
    PlasmaQuick::SharedQmlEngine *m_osdObject = nullptr;
    QTimer *m_osdTimer = nullptr;
    int m_timeout = 0;

    KConfigGroup m_osdConfigGroup;
};
