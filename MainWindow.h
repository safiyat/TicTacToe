#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>
#include <stdio.h>

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
		Q_OBJECT

	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();
	public slots:
		void runServer();
//		void stopServer();
		void connectToHost();
		void connectedToHost();
		void connectToClient();
		void send();
		void onDisconnected();
		void onReadyRead();
		void onServerConnect();
		void check(int i, int j);
		void gameStatus();

	private:
		Ui::MainWindow *ui;
		QTcpServer *server;
		QTcpSocket *socket;
		qint8 moves;
		int row, col;
		bool isserver;
		bool turn;
};

#endif // MAINWINDOW_H
