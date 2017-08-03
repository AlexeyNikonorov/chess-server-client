#include <QApplication>
#include "chess_gui.hpp"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	BoardWidget b;
	b.show();
	return app.exec();
}
