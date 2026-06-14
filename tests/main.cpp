#include "widget.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QDir>
#include <QQuickStyle>
#include "Logger.h"
#include "main/MainController.h"
#include <QQmlContext>

/**
 * @brief 初始化多分类日志系统，为每个日志分类创建独立的文件输出和控制台输出。
 */
void initAllLogging()
{
    QString logsDir = QDir::cleanPath(QString(PROJECT_ROOT) + "/data/logs");
    QDir().mkpath(logsDir);

    struct CategoryFile
    {
        LogCategory category;
        const char *fileName;
    };

    const CategoryFile categoryFiles[] = {
        {LogCategory::General,   "general.log"},
        {LogCategory::Database,  "database.log"},
        {LogCategory::Extract,   "extract.log"},
        {LogCategory::Ocr,       "ocr.log"},
        {LogCategory::Import,    "import.log"},
        {LogCategory::Export,    "export.log"},
        {LogCategory::UI,        "ui.log"},
        {LogCategory::Config,    "config.log"},
        {LogCategory::Network,   "network.log"},
        {LogCategory::Update,    "update.log"}
    };

    for (const auto &cf : categoryFiles)
    {
        QString filePath = logsDir + "/" + cf.fileName;
        auto fileTarget = QSharedPointer<FileTarget>::create(filePath);
        Logger::instance().addTarget(fileTarget, cf.category);
    }

    auto consoleTarget = QSharedPointer<ConsoleTarget>::create();
    Logger::instance().addTargetForAllCategories(consoleTarget);

    Logger::instance().setGlobalLevel(LogLevel::DEBUG);
    LOG_INFO_CAT(LogCategory::General) << "日志系统初始化完成，所有分类已注册";
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    initAllLogging();

    Widget widget;

    QQuickStyle::setStyle("Universal");

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { LOG_FATAL_CAT(LogCategory::General) << "QML对象创建失败"; },
        Qt::QueuedConnection);

    MainController controller;
    engine.rootContext()->setContextProperty("appController", &controller);

    const QUrl url = QUrl::fromLocalFile(PROJECTROOT + "/ui/main/main.qml");
    engine.load(url);

    return app.exec();
}