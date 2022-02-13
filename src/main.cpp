/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QCommandLineParser>
#include <QIcon>
#include <QImage>
#include <QAbstractListModel>

#include <KAboutData>
#include <KCrash>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <Option>

#include "DevicesModel.h"
#include "DocumentModel.h"
#include "FormatModel.h"
#include "FilteredOptionsModel.h"
#include "OCRLanguageModel.h"
#include "InProgressPainter.h"
#include "Skanpage.h"
#include "SkanpageUtils.h"
#include "skanpage_version.h"
#include "skanpage_config.h"
#include "skanpage_state.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication app(argc, argv);

    qmlRegisterUncreatableType<DocumentModel>("org.kde.skanpage", 1, 0, "DocumentModel", QStringLiteral("Document model class uncreateable"));
    qmlRegisterUncreatableType<DevicesModel>("org.kde.skanpage", 1, 0, "DevicesModel", QStringLiteral("Device model class uncreateable"));
    qmlRegisterUncreatableType<FormatModel>("org.kde.skanpage", 1, 0, "FormatModel", QStringLiteral("Format model class uncreateable"));
    qmlRegisterUncreatableType<FilteredOptionsModel>("org.kde.skanpage", 1, 0, "FilteredOptionsModel", QStringLiteral("Filtered options model class uncreateable"));
    qmlRegisterUncreatableType<OCRLanguageModel>("org.kde.skanpage", 1, 0, "OCRLanguageModel", QStringLiteral("OCR language model class uncreateable"));;
    qmlRegisterUncreatableType<KSaneCore::Option>("org.kde.skanpage", 1, 0, "KSaneOption", QStringLiteral("KSaneOption class uncreateable"));
    qmlRegisterUncreatableType<Skanpage>("org.kde.skanpage", 1, 0, "Skanpage", QStringLiteral("Skanpage application class uncreateable"));
    qmlRegisterUncreatableType<SkanpageConfiguration>("org.kde.skanpage", 1, 0, "Configuration", QStringLiteral("Confguration class uncreatable"));
    qmlRegisterUncreatableType<SkanpageState>("org.kde.skanpage", 1, 0, "StateConfiguration", QStringLiteral("State class uncreatable"));
    qmlRegisterUncreatableMetaObject(SkanpageUtils::staticMetaObject, "org.kde.skanpage", 1, 0, "SkanpageUtils", QStringLiteral("Error: only enums"));
    qmlRegisterType<InProgressPainter>("org.kde.skanpage", 1, 0, "InProgressPainter");
    qRegisterMetaType<SkanpageUtils::MessageLevel>();
    qRegisterMetaType<SkanpageUtils::FileType>();
    qRegisterMetaType<SkanpageUtils::PageProperties>();
    qRegisterMetaType<SkanpageUtils::DocumentPages>();
    qRegisterMetaType<QImage>();

    KLocalizedString::setApplicationDomain("skanpage");

    KAboutData aboutData(QLatin1String("skanpage"),
                         i18n("Skanpage"),
                         QLatin1String(SKANPAGE_VERSION_STRING),
                         i18n("Multi-page scanning application by KDE."),
                         KAboutLicense::GPL,
                         i18n("© 2015-2022 Kåre Särs, Alexander Stippich")
    );

    aboutData.addAuthor(QStringLiteral("Kåre Särs"), i18n("developer"), QLatin1String("kare.sars@iki.fi"));
    aboutData.addAuthor(QStringLiteral("Alexander Stippich"), i18n("developer"), QLatin1String("a.stippich@gmx.net"));
    aboutData.setBugAddress("https://bugs.kde.org/enter_bug.cgi?product=skanpage");

    app.setWindowIcon(QIcon::fromTheme(QLatin1String("skanpage")));

    KAboutData::setApplicationData(aboutData);
    KCrash::initialize();

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    QCommandLineOption deviceOption(
        QStringList({QLatin1String("d"), QLatin1String("device")}), i18n("Sane scanner device name. Use 'test' for test device."), i18n("device"));
    parser.addOption(deviceOption);
    parser.process(app);
    // the --author and --license is shown anyway but they work only with the following line
    aboutData.processCommandLine(&parser);

    const QString deviceName = parser.value(deviceOption);

    Skanpage skanpageApp = Skanpage(deviceName);

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.rootContext()->setContextProperty(QStringLiteral("skanpage"), &skanpageApp);
    engine.rootContext()->setContextProperty(QStringLiteral("_aboutData"), QVariant::fromValue(aboutData));
    engine.load(QUrl(QStringLiteral("qrc:/qml/MainWindow.qml")));

    return app.exec();
}
