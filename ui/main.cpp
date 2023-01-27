#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtCore>
#include <QtGui>
#include <QtQml>
#include <QtWidgets>
#include "t3traincontroller.hpp"
int main(int argc, char *argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
	QApplication app(argc, argv);

	QQmlApplicationEngine engine;
	const QUrl url(QStringLiteral("qrc:/main.qml"));
	QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
	&app, [url](QObject * obj, const QUrl & objUrl) {
		if (!obj && url == objUrl)
			QCoreApplication::exit(-1);
	}, Qt::QueuedConnection);

	T3TrainController testTrainController;
	engine.rootContext()->setContextProperty("testTrainController", &testTrainController);

	engine.load(url);


	testTrainController.startTimer(100);

	QFontDatabase::addApplicationFont(":/T3InterFont.ttf");
	QFontDatabase::addApplicationFont(":/T3SegFont.ttf");

	return app.exec();
}