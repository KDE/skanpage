/* ============================================================
 *
 * Copyright (C) 2015 by Kåre Särs <kare.sars@iki .fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License.
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * ============================================================ */
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QIcon>

#include <KAboutData>
#include <KLocalizedString>
#include <KI18n/KLocalizedContext>

#include "DocumentModel.h"
#include "Skanpage.h"
#include "SaveFileDialog.h"
#include "skanpage_version.h"

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    
    QApplication app(argc, argv);

    qmlRegisterType<DocumentModel> ("org.kde.skanpage", 1, 0, "DocumentModel");
    qmlRegisterType<SaveFileDialog>("org.kde.skanpage", 1, 0, "SaveFileDialog");


    KLocalizedString::setApplicationDomain("skanlite");

    KAboutData aboutData(QLatin1String("Skanpage"), // componentName, k4: appName
                         i18n("Skanpage"), // displayName, k4: programName
                         QLatin1String(SKANPAGE_VERSION_STRING), // version
                         i18n("Page-scanning application for KDE based on libksane."), // shortDescription
                         KAboutLicense::GPL, // licenseType
                         i18n("(C) 2015 Kåre Särs"), // copyrightStatement
                         QString(), // other Text
                         QString() // homePageAddress
    );

    aboutData.addAuthor(i18n("Kåre Särs"),
                        i18n("developer"),
                        QLatin1String("kare.sars@iki.fi"));

    app.setWindowIcon(QIcon::fromTheme(QLatin1String("scanner")));

    QCoreApplication::setApplicationVersion(aboutData.version());


    
    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption deviceOption(QStringList() << QLatin1String("d") << QLatin1String("device"), i18n("Sane scanner device name. Use 'test' for test device."), i18n("device"));
    parser.addOption(deviceOption);
    parser.process(app); // the --author and --license is shown anyway but they work only with the following line
    aboutData.processCommandLine(&parser);

    const QString deviceName = parser.value(deviceOption);
    //qDebug() << QString::fromLatin1("deviceOption value=%1").arg(deviceName);
    
   
    Skanpage skanpageApp = Skanpage(deviceName);
    skanpageApp.setAboutData(&aboutData);  
    
    QQmlApplicationEngine engine;
    
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.rootContext()->setContextProperty(QStringLiteral("skanPage"), &skanpageApp);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
