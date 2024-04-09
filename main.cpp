#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QQuickItem>
#include <QQuickWindow>
#include <QQmlContext>
#include <QRunnable>
#include <gst/gst.h>
#include <thread>

#include "camera_gst.h"
#include "tester.h"
#include "logger.h"
#include "log_duration.h"


class SetPlaying : public QRunnable
{
public:
  SetPlaying(GstElement *);
  ~SetPlaying();

  void run ();

private:
  GstElement * pipeline_;
};

SetPlaying::SetPlaying (GstElement * pipeline)
{
  this->pipeline_ = pipeline ? static_cast<GstElement *> (gst_object_ref (pipeline)) : NULL;
}

SetPlaying::~SetPlaying ()
{
  if (this->pipeline_)
    gst_object_unref (this->pipeline_);
}

void
SetPlaying::run ()
{
  if (this->pipeline_)
    gst_element_set_state (this->pipeline_, GST_STATE_PLAYING);
}

int main(int argc, char *argv[])
{
    // std::cout << "FFFFFFFFFFFFF" << std::endl;
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    CameraGST camera_(&argc, argv);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QQmlContext *ctx = engine.rootContext();
    // Logger::init();

    Tester tester = Tester(camera_);
    tester.init();

    ctx->setContextProperty("tester", &tester);

    qmlRegisterUncreatableType<Tester>
            ("macro.tester", 1, 0, "Tester",
             QStringLiteral("Tester should not be created in QML"));

    // const QUrl url(QStringLiteral("/usr/local/share/main.qml"));
    const QUrl url(QStringLiteral("../main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);
    
    const QQuickWindow *rootObject = static_cast<QQuickWindow *> (engine.rootObjects().first());
    QQuickItem *videoItem = rootObject->findChild<QQuickItem *> ("videoItem");
    camera_.ObjectSet(camera_.GetSinkElement(), "widget", videoItem, NULL);

    

    QObject::connect(rootObject, &QQuickWindow::sceneGraphInitialized,
                     &tester, [&](void) {
        // bool res = camera_.Init();
        // if( res )
        // {
        //   std::cout << "Camera init OK" << std::endl;
        // } else 
        // {
        //   std::cout << "Camera init FAIL" << std::endl;
        // }
    }, Qt::QueuedConnection);

    
    // QQuickItem *videoItem;
    // QQuickWindow *rootObject;

    /* find and set the videoItem on the sink */
    // rootObject = static_cast<QQuickWindow *> (engine.rootObjects().first());
    // videoItem = rootObject->findChild<QQuickItem *> ("videoItem");
    // g_assert (videoItem);
    // camera_.ObjectSet(camera_.GetSinkElement(), "widget", videoItem, NULL);
    // g_object_set(sink, "widget", videoItem, NULL);

    // rootObject->scheduleRenderJob (new SetPlaying (pipeline),
    //     QQuickWindow::BeforeSynchronizingStage);
    // const QUrl url(QStringLiteral("../main.qml"));
    
    // camera_.CameraPlay();

    auto ret = app.exec();
    return ret;
}