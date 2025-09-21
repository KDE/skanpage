/**
 * SPDX-FileCopyrightText: 2024 by Milena Cole <mkoul@mail.ru>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "NameTemplateUtils.h"

#include "FormatModel.h"
#include "skanpage_config.h"

#include <QDateTime>
#include <QFile>
#include <QRegularExpression>

NameTemplateUtils::NameTemplateUtils(QObject* parent):
    QObject(parent)
{

}

QString NameTemplateUtils::addSampleToText(QString text, const QString &sample, int ind)
{
    text.insert(ind, sample);
    return text;
}

QString NameTemplateUtils::getSample(int i)
{
    if (i < m_samples.count()) {
        return std::get<0>(m_samples[i]);
    }
    return QStringLiteral("");
}

QString NameTemplateUtils::getSampleName(int i)
{
    if (i < m_samples.count()) {
        return std::get<1>(m_samples[i]);
    }
    return QStringLiteral("");
}

QUrl NameTemplateUtils::createUnparsedFileUrl()
{
    SkanpageConfiguration *configuration = SkanpageConfiguration::self();

    QString folder = configuration->defaultFolder().toString();
    QString name = configuration->nameTemplate();
    QString suffix = FormatModel().getData(configuration->defaultNameFilterIndex(), FormatModel::SuffixRole).toString();

    return QUrl(folder + QLatin1Char('/') + name + QLatin1Char('.') + suffix);
}

QUrl NameTemplateUtils::fileUrl(QString path, bool extraNumeric)
{
    QString suffix = path.right(path.length() - path.lastIndexOf(QRegularExpression(QStringLiteral("\\.\\w+"))));
    path = parseSamples(path).remove(suffix);
    if (extraNumeric) {
        path += GetExtraNumeric(path, suffix);
    }
    if (!path.startsWith(QStringLiteral("file://")))
        path.push_front(QStringLiteral("file://"));
    return QUrl(path + suffix);
}

QUrl NameTemplateUtils::fileUrl(bool extraNumeric)
{
    return fileUrl(createUnparsedFileUrl().path(), extraNumeric);
}

QString NameTemplateUtils::parseSamples(QString path)
{
    path.remove(QStringLiteral("file://"));
    SampleType samples = NameTemplateUtils().m_samples;
    for (int i = 0; i < samples.count(); ++i) {
        if (path.contains(std::get<0>(samples[i]))) {
            path.replace(std::get<0>(samples[i]), std::get<2>(samples[i])());
        }
    }
    return path;
}

QString NameTemplateUtils::parseSamples(QString folder, QString name, QString format)
{
    QString path = folder + QLatin1Char('/') + name + QLatin1Char('.') + format;
    return parseSamples(path);
}

QString NameTemplateUtils::getYear()
{
    return QString::number(QDateTime::currentDateTime().date().year());
}

QString NameTemplateUtils::getShortYear()
{
    return QString::number(QDateTime::currentDateTime().date().year()).right(2);
}

QString NameTemplateUtils::getMonth()
{
    return QString::number(QDateTime::currentDateTime().date().month());
}

QString NameTemplateUtils::getDay()
{
    return QString::number(QDateTime::currentDateTime().date().day());
}

QString NameTemplateUtils::getHour()
{
    return QString::number(QDateTime::currentDateTime().time().hour());
}

QString NameTemplateUtils::getMinute()
{
    return QString::number(QDateTime::currentDateTime().time().minute());
}

QString NameTemplateUtils::getSecond()
{
    return QString::number(QDateTime::currentDateTime().time().second());
}

QString NameTemplateUtils::getFullDate()
{
    return getYear() + QLatin1Char('_') + getMonth() + QLatin1Char('_') + getDay();
}

QString NameTemplateUtils::getFullTime()
{
    return getHour() + QLatin1Char('-') + getMinute() + QLatin1Char('-') + getSecond();
}

QString NameTemplateUtils::iteratorToString(int i, int n)
{
    QString iStr = QString::number(i).rightJustified(n, QLatin1Char('0'));
    return iStr;
}

QString NameTemplateUtils::iteratorToString(int i)
{
    return QStringLiteral(" (") + QString::number(i) + QLatin1Char(')');
}

QString NameTemplateUtils::copySuffix(const QString &path, const QString &suffix)
{
    if (!QFile::exists(path + suffix)) {
        return QStringLiteral("");
    }
    int iterator = 0;
    QString iterStr;
    do {
        ++iterator;
        iterStr = iteratorToString(iterator);
    } while (QFile::exists(path + iterStr + suffix));
    return iterStr;
}

bool NameTemplateUtils::isContainsIncrementNumber(const QString &path)
{
    return path.contains(QRegularExpression(QStringLiteral("\\$\\[n+\\]")));
}

QString NameTemplateUtils::GetExtraNumeric(const QString &path, const QString &suffix)
{
    int iterator = -1; // for start from _0000
    QString iterStr;
    do {
        ++iterator;
        iterStr = QLatin1Char('_') + iteratorToString(iterator, 4);
    } while (QFile::exists(path + iterStr + suffix));
    return iterStr;
}

bool NameTemplateUtils::isFileExists(QString path, bool extraNumeric)
{
    path.remove(QStringLiteral("file://"));
    QString suffix = path.right(path.length() - path.lastIndexOf(QRegularExpression(QStringLiteral("\\.\\w+"))));
    path = parseSamples(path).remove(suffix);
    if (extraNumeric) {
        path += GetExtraNumeric(path, suffix);
    }
    return QFile::exists(path+suffix);
}

bool NameTemplateUtils::isFileExists(const QString &path, QString name, const QString &suffix, bool extraNumeric)
{
    return isFileExists(path + name + QLatin1Char('.') + suffix, extraNumeric);
}

#include "moc_NameTemplateUtils.cpp"
