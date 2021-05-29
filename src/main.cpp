/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 *  
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QtQml>

#include <KAboutData>
#include <KI18n/KLocalizedContext>
#include <KLocalizedString>
#include <KSaneOption>

#include "DevicesModel.h"
#include "DocumentModel.h"
#include "OptionsModel.h"
#include "SingleOption.h"
#include "Skanpage.h"
#include "skanpage_version.h"

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);

    qmlRegisterUncreatableType<DocumentModel>("org.kde.skanpage", 1, 0, "DocumentModel", QStringLiteral("Document model class uncreateable"));
    qmlRegisterUncreatableType<DevicesModel>("org.kde.skanpage", 1, 0, "DevicesModel", QStringLiteral("Device model class uncreateable"));
    qmlRegisterUncreatableType<OptionsModel>("org.kde.skanpage", 1, 0, "OptionsModel", QStringLiteral("Options model class uncreateable"));
    qmlRegisterUncreatableType<SingleOption>("org.kde.skanpage", 1, 0, "SingleOption", QStringLiteral("Single option class uncreateable"));
    qmlRegisterUncreatableType<KSaneOption>("org.kde.skanpage", 1, 0, "KSaneOption", QStringLiteral("KSaneOption class uncreateable"));
    qmlRegisterUncreatableType<Skanpage>("org.kde.skanpage", 1, 0, "Skanpage", QStringLiteral("Skanpage application class uncreateable"));
    
    KLocalizedString::setApplicationDomain("skanpage");

    KAboutData aboutData(QLatin1String("Skanpage"), // componentName, k4: appName
                         i18n("Skanpage"), // displayName, k4: programName
                         QLatin1String(SKANPAGE_VERSION_STRING), // version
                         i18n("Page-scanning application by KDE based on libksane."), // shortDescription
                         KAboutLicense::GPL, // licenseType
                         i18n("(C) 2015 Kåre Särs"), // copyrightStatement
                         QString(), // other Text
                         QString() // homePageAddress
    );

    aboutData.addAuthor(QStringLiteral("Kåre Särs"), i18n("developer"), QLatin1String("kare.sars@iki.fi"));
    aboutData.addAuthor(QStringLiteral("Alexander Stippich"), i18n("developer"), QLatin1String("a.stippich@gmx.net"));
    
    app.setWindowIcon(QIcon::fromTheme(QLatin1String("scanner")));

    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption deviceOption(
        QStringList() << QLatin1String("d") << QLatin1String("device"), i18n("Sane scanner device name. Use 'test' for test device."), i18n("device"));
    parser.addOption(deviceOption);
    parser.process(app); // the --author and --license is shown anyway but they work only with the following line
    aboutData.processCommandLine(&parser);

    const QString deviceName = parser.value(deviceOption);

    Skanpage skanpageApp = Skanpage(deviceName);

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.rootContext()->setContextProperty(QStringLiteral("skanpage"), &skanpageApp);
    engine.load(QUrl(QStringLiteral("qrc:/MainWindow.qml")));

    return app.exec();
}
