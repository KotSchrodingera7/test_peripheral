#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QQmlContext>

#include <thread>

#include "tester.h"
#include "logger.h"

int main(int argc, char *argv[])
{
    // std::cout << "FFFFFFFFFFFFF" << std::endl;
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    
    // std::cout << argv[1] << std::endl;
    
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QQmlContext *ctx = engine.rootContext();
    Logger::init();
    
    Tester *tester = new Tester();
    tester->init();

    ctx->setContextProperty("tester", tester);

    qmlRegisterUncreatableType<Tester>
            ("macro.tester", 1, 0, "Tester",
             QStringLiteral("Tester should not be created in QML"));

    const QUrl url(QStringLiteral("/usr/local/share/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}