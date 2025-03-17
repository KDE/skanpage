/**
 * SPDX-FileCopyrightText: 2024 by Milena Cole <mkoul@mail.ru>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef NAMETEMPLATEUTILS_H
#define NAMETEMPLATEUTILS_H

#include <KLocalizedString>

#include <QObject>

typedef QList<std::tuple<QString, QString, std::function<QString()>>> SampleType; // <sample, description, parse function>

class NameTemplateUtils: public QObject
{
    Q_OBJECT
private:
    // ----------------------------------------------------- SAMPLES LIST ----------------------------------------------------- //
    //                                     Sample                 Description                              Parse function

    SampleType m_samples {
                          {QStringLiteral("$[YYYY]"),             i18n("Year"),                             getYear},
                          {QStringLiteral("$[YY]"),               i18n("Year (00–99)"),                     getShortYear},
                          {QStringLiteral("$[MM]"),               i18n("Month"),                            getMonth},
                          {QStringLiteral("$[DD]"),               i18n("Day"),                              getDay},
                          {QStringLiteral("$[hh]"),               i18n("Hour"),                             getHour},
                          {QStringLiteral("$[mm]"),               i18n("Minute"),                           getMinute},
                          {QStringLiteral("$[ss]"),               i18n("Second"),                           getSecond},
                          {QStringLiteral("$[YYYY]-$[MM]-$[DD]"), i18n("Full date"),                        getFullDate},
                          {QStringLiteral("$[hh]_$[mm]_$[ss]"),   i18n("Full time"),                        getFullTime},
    };
    // ------------------------------------------------------------------------------------------------------------------------ //

    // ----- function to parse sample ------- //
    static QString getYear();
    static QString getShortYear();
    static QString getMonth();
    static QString getDay();
    static QString getHour();
    static QString getMinute();
    static QString getSecond();
    static QString getFullDate();
    static QString getFullTime();
    // ---------------------------------------- //
    static QString iteratorToString(int i, int n); // format: dddd (0001, 0002, 0003 ....)
    static QString iteratorToString(int i); // format: (d) ( (1), (2), (3) ...)
    static QString copySuffix(const QString &path, const QString &suffix); // suffix of copy of this document ( s (1).pdf, s (2).pdf ...)
    static QString GetExtraNumeric(const QString &path, const QString &suffix); // if the pages are divided into several documents. format: _dddd (_0000, _0001, _0002 ...)

public:
    NameTemplateUtils(QObject* parent = nullptr);
    Q_INVOKABLE QString addSampleToText(QString text, const QString &sample, int ind = 0); // add sample to templ text
    Q_INVOKABLE QString getSample(int i); // return sample from samples list
    Q_INVOKABLE QString getSampleName(int i); // return sample description
    Q_INVOKABLE QUrl createUnparsedFileUrl();
    Q_INVOKABLE QUrl fileUrl(QString path, bool extraNumeric = false); //parse file path and return file url
    Q_INVOKABLE QUrl fileUrl(bool extraNumeric = false); //parse file path and return file url
    Q_INVOKABLE static QString parseSamples(QString path);
    Q_INVOKABLE static QString parseSamples(QString folder, QString name, QString format);
    Q_INVOKABLE static bool isContainsIncrementNumber(const QString &path); // does path have increment number samples
    Q_INVOKABLE static bool isFileExists(QString path, bool extraNumeric = false);
    Q_INVOKABLE static bool isFileExists(const QString &path, QString name, const QString &suffix, bool extraNumeric = false);

};

#endif // NAMETEMPLATEUTILS_H
