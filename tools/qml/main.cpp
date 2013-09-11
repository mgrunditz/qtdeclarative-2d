/****************************************************************************
**
** Copyright (C) 2013 Research In Motion.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "conf.h"

#include <QCoreApplication>
#include <QGuiApplication>
#ifdef QT_WIDGETS_LIB
#include <QApplication>
#endif
#include <QWindow>
#include <QQmlApplicationEngine>
#include <QFileOpenEvent>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStringList>
#include <QDebug>
#include <QStandardPaths>
#include <QtGlobal>
#include <qqml.h>
#include <qqmldebug.h>
#include <private/qabstractanimation_p.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>

#define VERSION_MAJ 1
#define VERSION_MIN 0
#define VERSION_STR "1.0"

static Config *conf = 0;
static QQmlApplicationEngine *qae = 0;

static void loadConf(const QString &override, bool quiet) // Terminates app on failure
{
    const QString defaultFileName = QLatin1String("configuration.qml");
    QUrl settingsUrl;
    bool builtIn = false; //just for keeping track of the warning
    if (override.isEmpty()) {
        QFileInfo fi;
        fi.setFile(QStandardPaths::locate(QStandardPaths::DataLocation, defaultFileName));
        if (fi.exists()) {
            settingsUrl = QUrl::fromLocalFile(fi.absoluteFilePath());
        } else {
            // ### If different built-in configs are needed per-platform, just apply QFileSelector to the qrc conf.qml path
            settingsUrl = QUrl(QLatin1String("qrc:///qt-project.org/QmlRuntime/conf/") + defaultFileName);
            builtIn = true;
        }
    } else {
        QFileInfo fi;
        fi.setFile(override);
        if (!fi.exists()) {
            qCritical() << QObject::tr("qml: Couldn't find required configuration file:") << fi.absoluteFilePath();
            exit(1);
        }
        settingsUrl = QUrl::fromLocalFile(fi.absoluteFilePath());
    }

    if (!quiet) {
        if (builtIn)
            qWarning() << QObject::tr("qml: Using built-in configuration.");
        else
            qWarning() << QObject::tr("qml: Using configuration file:") << settingsUrl;
    }

    // TODO: When we have better engine control, ban QtQuick* imports on this engine
    QQmlEngine e2;
    QQmlComponent c2(&e2, settingsUrl);
    conf = qobject_cast<Config*>(c2.create());

    if (!conf){
        qCritical() << QObject::tr("qml: Error loading configuration file:") << c2.errorString();
        exit(1);
    }
}

void contain(QObject *o, const QUrl &containPath)
{
    QQmlComponent c(qae, containPath);
    QObject *o2 = c.create();
    if (!o2)
        return;
    bool success = false;
    int idx;
    if ((idx = o2->metaObject()->indexOfProperty("containedObject")) != -1)
        success = o2->metaObject()->property(idx).write(o2, QVariant::fromValue<QObject*>(o));
    if (!success)
        o->setParent(o2); //Set QObject parent, and assume container will react as needed
}

// Loads qml after receiving a QFileOpenEvent
class LoaderApplication : public QGuiApplication
{
public:
    LoaderApplication(int& argc, char **argv) : QGuiApplication(argc, argv) {}

    bool event(QEvent *ev)
    {
        if (ev->type() == QEvent::FileOpen)
            qae->load(static_cast<QFileOpenEvent *>(ev)->url());
        else
            return QGuiApplication::event(ev);
        return true;
    }
};

// Listens to the appEngine signals to determine if all files failed to load
class LoadWatcher : public QObject
{
    Q_OBJECT
public:
    LoadWatcher(QQmlApplicationEngine *e, int expected)
        : QObject(e)
        , expect(expected)
        , haveOne(false)
    {
        connect(e, SIGNAL(objectCreated(QObject*,QUrl)),
            this, SLOT(checkFinished(QObject*)));
    }

private:
    int expect;
    bool haveOne;

public Q_SLOTS:
    void checkFinished(QObject *o)
    {
        if (o) {
            haveOne = true;
            if (conf && qae)
                foreach (PartialScene *ps, conf->completers)
                    if (o->inherits(ps->itemType().toUtf8().constData()))
                        contain(o, ps->container());
        }
        if (haveOne)
            return;

        if (! --expect) {
            qCritical() << QObject::tr("qml: Did not load any objects, exiting.");
            exit(2);//Different return code from qFatal
        }
    }
};

void quietMessageHandler(QtMsgType type, const QMessageLogContext &ctxt, const QString &msg)
{
    Q_UNUSED(ctxt);
    Q_UNUSED(msg);
    //Doesn't print anything
    switch (type) {
    case QtFatalMsg:
        abort();
    case QtCriticalMsg:
    case QtDebugMsg:
    case QtWarningMsg:
        ;
    }
}


// ### Should command line arguments have translations? Qt creator doesn't, so maybe it's not worth it.
bool useCoreApp = false;
bool useWidgetApp = false;
bool quietMode = false;
void printVersion()
{
    printf("qml binary version ");
    printf(VERSION_STR);
    printf("\nbuilt with Qt version ");
    printf(QT_VERSION_STR);
    printf("\n");
    exit(0);
}

void printUsage()
{
    printf("Usage: qml [options] [files]\n");
    printf("\n");
    printf("Any argument ending in .qml will be treated as a QML file to be loaded.\n");
    printf("Any number of QML files can be loaded. They will share the same engine.\n");
    printf("Any argument which is not a recognized option and which does not end in .qml will be ignored.\n");
    printf("'widget' application type is only available if the QtWidgets module is avaialble.\n");
    printf("\n");
    printf("General Options:\n");
    printf("\t-h, -help..................... Print this usage information and exit.\n");
    printf("\t-v, -version.................. Print the version information and exit.\n");
    printf("\t-apptype [core|gui|widget] ... Select which application class to use. Default is gui.\n");
    printf("\t-quiet ....................... Suppress all output.\n");
    printf("\t-I [path] .................... Prepend the given path to the import paths.\n");
    printf("\t-f [file] .................... Load the given file as a QML file.\n");
    printf("\t-config [file] ............... Load the given file as the configuration file.\n");
    printf("\t-- ........................... Arguments after this one are ignored by the launcher, but may be used within the QML application.\n");
    printf("\tDebugging options:\n");
    printf("\t-enable-debugger ............. Allow the QML debugger to connect to the application (also requires debugger arguments).\n");
    printf("\t-translation [file] .......... Load the given file as the translations file.\n");
    printf("\t-dummy-data [directory] ...... Load QML files from the given directory as context properties.\n");
    printf("\t-slow-animations ............. Run all animations in slow motion.\n");
    printf("\t-fixed-animations ............ Run animations off animation tick rather than wall time.\n");
    exit(0);
}

//Called before application initialization, removes arguments it uses
void getAppFlags(int &argc, char **argv)
{
    for (int i=0; i<argc; i++) {
        if (!strcmp(argv[i], "-apptype")) { // Must be done before application, as it selects application
            int type = 0;
            if (i+1 < argc) {
                if (!strcmp(argv[i+1], "core"))
                    type = 1;
                else if (!strcmp(argv[i+1], "gui"))
                    type = 2;
#ifdef QT_WIDGETS_LIB
                else if (!strcmp(argv[i+1], "widget"))
                    type = 3;
#endif
            }

            if (!type) {
#ifdef QT_WIDGETS_LIB
                printf("-apptype must be followed by one of the following: core gui widget\n");
#else
                printf("-apptype must be followed by one of the following: core gui\n");
#endif
                printUsage();
            }

            switch (type) {
            case 1: useCoreApp = true; break;
            case 2: useCoreApp = false; break;
#ifdef QT_WIDGETS_LIB
            case 3: useWidgetApp = true; break;
#endif
            }
            for (int j=i; j<argc-2; j++)
                argv[j] = argv[j+2];
            argc -= 2;
        } else if (!strcmp(argv[i], "-enable-debugger")) { // Normally done via a define in the include, so expects to be before application (and must be before engine)
            static QQmlDebuggingEnabler qmlEnableDebuggingHelper(true);
            for (int j=i; j<argc-1; j++)
                argv[j] = argv[j+1];
            argc --;
        }
    }
}

void getFileSansBangLine(const QString &path, QByteArray &output)
{
    QFile f(path);
    if (!f.open(QFile::ReadOnly | QFile::Text))
        return;
    output = f.readAll();
    if (output.startsWith("#!"))//Remove first line in this case (except \n, to avoid disturbing line count)
        output.remove(0, output.indexOf('\n'));
}

static void loadDummyDataFiles(QQmlEngine &engine, const QString& directory)
{
    QDir dir(directory+"/dummydata", "*.qml");
    QStringList list = dir.entryList();
    for (int i = 0; i < list.size(); ++i) {
        QString qml = list.at(i);
        QFile f(dir.filePath(qml));
        f.open(QIODevice::ReadOnly);
        QByteArray data = f.readAll();
        QQmlComponent comp(&engine);
        comp.setData(data, QUrl());
        QObject *dummyData = comp.create();

        if (comp.isError()) {
            QList<QQmlError> errors = comp.errors();
            foreach (const QQmlError &error, errors)
                qWarning() << error;
        }

        if (dummyData && !quietMode) {
            qWarning() << QObject::tr("qml: Loaded dummy data:") << dir.filePath(qml);
            qml.truncate(qml.length()-4);
            engine.rootContext()->setContextProperty(qml, dummyData);
            dummyData->setParent(&engine);
        }
    }
}

int main(int argc, char *argv[])
{
    getAppFlags(argc, argv);
    QCoreApplication *app;
    if (useCoreApp)
        app = new QCoreApplication(argc, argv);
#ifdef QT_WIDGETS_LIB
    else if (useWidgetApp)
        app = new QApplication(argc, argv);
#endif
    else
        app = new LoaderApplication(argc, argv);

    app->setApplicationName("Qml Runtime");
    app->setOrganizationName("Qt Project");
    app->setOrganizationDomain("qt-project.org");

    qmlRegisterType<Config>("QmlRuntime.Config", VERSION_MAJ, VERSION_MIN, "Configuration");
    qmlRegisterType<PartialScene>("QmlRuntime.Config", VERSION_MAJ, VERSION_MIN, "PartialScene");
    QQmlApplicationEngine e;
    QStringList files;
    QString confFile;
    QString translationFile;
    QString dummyDir;

    //Handle main arguments
    QStringList argList = app->arguments();
    for (int i = 0; i < argList.count(); i++) {
        const QString &arg = argList[i];
        if (arg == QLatin1String("-quiet"))
            quietMode = true;
        else if (arg == QLatin1String("-v") || arg == QLatin1String("-version"))
            printVersion();
        else if (arg == QLatin1String("-h") || arg == QLatin1String("-help"))
            printUsage();
        else if (arg == QLatin1String("--"))
            break;
        else if (arg == QLatin1String("-slow-animations"))
            QUnifiedTimer::instance()->setSlowModeEnabled(true);
        else if (arg == QLatin1String("-fixed-animations"))
            QUnifiedTimer::instance()->setConsistentTiming(true);
        else if (arg == QLatin1String("-I")) {
            if (i+1 == argList.count())
                continue;//Invalid usage, but just ignore it
            e.addImportPath(argList[i+1]);
            i++;
        } else if (arg == QLatin1String("-f")) {
            if (i+1 == argList.count())
                continue;//Invalid usage, but just ignore it
            files << argList[i+1];
            i++;
        } else if (arg == QLatin1String("-config")){
            if (i+1 == argList.count())
                continue;//Invalid usage, but just ignore it
            confFile = argList[i+1];
            i++;
        } else if (arg == QLatin1String("-translation")){
            if (i+1 == argList.count())
                continue;//Invalid usage, but just ignore it
            translationFile = argList[i+1];
            i++;
        } else if (arg == QLatin1String("-dummy-data")){
            if (i+1 == argList.count())
                continue;//Invalid usage, but just ignore it
            dummyDir = argList[i+1];
            i++;
        } else {
            //If it ends in .qml, treat it as a file. Else ignore it
            if (arg.endsWith(".qml"))
                files << arg;
        }
    }

#ifndef QT_NO_TRANSLATION
    //qt_ translations loaded by QQmlApplicationEngine
    QTranslator qmlTranslator;
    QString sysLocale = QLocale::system().name();
    if (qmlTranslator.load(QLatin1String("qml_") + sysLocale, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app->installTranslator(&qmlTranslator);

    if (!translationFile.isEmpty()) { //Note: installed before QQmlApplicationEngine's automatic translation loading
        QTranslator translator;

        if (translator.load(translationFile)) {
            app->installTranslator(&translator);
        } else {
            if (!quietMode)
                qWarning() << "qml: Could not load the translation file" << translationFile;
        }
    }
#else
    if (!translationFile.isEmpty() && !quietMode)
        qWarning() << "qml: Translation file specified, but Qt built without translation support.";
#endif

    if (quietMode)
        qInstallMessageHandler(quietMessageHandler);

    if (files.count() <= 0) {
        if (!quietMode)
            qCritical() << QObject::tr("qml: No files specified. Terminating.");
        exit(1);
    }

    qae = &e;
    loadConf(confFile, quietMode);

    //Load files
    LoadWatcher lw(&e, files.count());

    foreach (const QString &path, files) {
        //QUrl::fromUserInput doesn't treat no scheme as relative file paths
        QRegularExpression urlRe("[[:word:]]+://.*");
        if (urlRe.match(path).hasMatch()) { //Treat as a URL
            QUrl url = QUrl::fromUserInput(path);
            if (!quietMode)
                qDebug() << QObject::tr("qml: loading ") << url;
            e.load(url);
        } else { //Local file path
            if (!quietMode) {
                qDebug() << QObject::tr("qml: loading ") << path;
                QByteArray strippedFile;
                getFileSansBangLine(path, strippedFile);
                if (strippedFile.isEmpty())
                    // If there's an error opening the file, this will give us the right error message
                    e.load(path);
                else
                    e.loadData(strippedFile, QUrl::fromLocalFile(path));
            } else {
                e.load(path);
            }
        }
    }


    if (!dummyDir.isEmpty() && QFileInfo (dummyDir).isDir())
        loadDummyDataFiles(e, dummyDir);

    return app->exec();
}

#include "main.moc"