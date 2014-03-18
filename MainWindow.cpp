#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->gameBox->setEnabled(FALSE);
	connect(ui->clientButton, SIGNAL(released()), this, SLOT(connectToHost()));
	connect(ui->serverButton, SIGNAL(released()), this, SLOT(runServer()));
	moves = 0;
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::runServer()
{
	this->setWindowTitle("Player1");
	server = new QTcpServer(this);
	if(!server->listen(QHostAddress::Any, 5555))
	{
		ui->statusBar->showMessage("Unable to start game server.");
		return;
	}
	ui->connectBox->setEnabled(FALSE);
	ui->statusBar->showMessage("Game Server running. Waiting for other player...");
	ui->gameBox->setEnabled(TRUE);
	ui->playButton->setEnabled(FALSE);
	isserver = TRUE;
	connect(server, SIGNAL(newConnection()), this, SLOT(connectToClient()));
}

void MainWindow::connectToHost()
{
	this->setWindowTitle("Player2");
	socket = new QTcpSocket(this);
	socket->connectToHost(ui->ipEdit->text(), 5555);
	ui->connectBox->setEnabled(FALSE);
	ui->statusBar->showMessage("Connecting to Player1...");
	connect(socket, SIGNAL(connected()), this, SLOT(connectedToHost()));
}

void MainWindow::connectedToHost()
{
	isserver = FALSE;
	ui->statusBar->showMessage("Connected to Player1. Waiting for Player1's move...");
	turn = FALSE;
	ui->gameBox->setEnabled(TRUE);
	ui->playButton->setEnabled(FALSE);
//	connect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(gameStatus()));
	connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

void MainWindow::connectToClient()
{
	socket = server->nextPendingConnection();
	ui->statusBar->showMessage("Connecting to Player2...");
	while(socket->state() == QTcpSocket::ConnectingState)
	{
	}

	if(socket->state() == QTcpSocket::ConnectedState)
	{
		ui->statusBar->showMessage("Player2 connected. Begin game!");
		turn = TRUE;
		ui->playButton->setEnabled(TRUE);
		connect(ui->playButton, SIGNAL(released()), this, SLOT(send()));
		connect(ui->tableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(check(int,int)));
//		connect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(gameStatus()));
	}
	else
	{
		ui->statusBar->showMessage("Connection to Player2 failed.");
		socket->deleteLater();
		return;
	}
}

void MainWindow::send()
{
	QString player;
	if(isserver)
		player = "X";
	else
		player = "O";
	ui->tableWidget->setItem(row, col, new QTableWidgetItem(player));
	ui->playButton->setEnabled(FALSE);
	socket->write(QString(tr("%1 %2").arg(row).arg(col)).toAscii());
	socket->flush();
	moves++;
	ui->statusBar->showMessage("Waiting for the other player...");
	gameStatus();
	connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	disconnect(ui->playButton, SIGNAL(released()), this, SLOT(send()));
	turn = FALSE;
}

void MainWindow::onDisconnected()
{
	connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	ui->statusBar->showMessage("Disconnected with the other player.");
	ui->tableWidget->clearContents();
	ui->gameBox->setEnabled(FALSE);
	ui->connectBox->setEnabled(TRUE);
	socket->deleteLater();
}

void MainWindow::onReadyRead()
{
	char *message;
	message = new char[5];
	socket->read(message, 5);
	socket->flush();
	sscanf(message, "%d%d", &row, &col);
	QString player;
	if(isserver)
		player = "O";
	else
		player = "X";
	moves++;
	ui->tableWidget->setItem(row, col, new QTableWidgetItem(player));
	ui->playButton->setEnabled(TRUE);
	ui->statusBar->showMessage("Our turn. Make move.");
	gameStatus();
	turn = TRUE;
	disconnect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	connect(ui->tableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(check(int,int)));
	connect(ui->playButton, SIGNAL(released()), this, SLOT(send()));
}

void MainWindow::onServerConnect()
{

}

void MainWindow::check(int i, int j)
{
	/*
	row = i;
	col = j;
	*/
	if(!turn)
		return;

	if(!ui->tableWidget->item(i, j)/* || ui->tableWidget->item(i, j)->text().isNull()*/)
	{
		ui->playButton->setEnabled(TRUE);
		ui->statusBar->showMessage("");
		row = i;
		col = j;
//		ui->statusBar->showMessage(tr("%1, %2").arg(row).arg(col));
	}
	else
	{
		ui->playButton->setEnabled(FALSE);
		ui->tableWidget->item(i, j)->setSelected(FALSE);
		ui->statusBar->showMessage("Cannot mark a played position.");
//		ui->statusBar->showMessage(tr("%1, %2").arg(row).arg(col));
	}
}

void MainWindow::gameStatus()
{
	QString mat[3][3];
	QString winner;
	if(moves < 5)
		return;
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 3; j++)
			if(ui->tableWidget->item(i, j))
				mat[i][j] = ui->tableWidget->item(i, j)->text();
			else
				mat[i][j] = tr("%1%2").arg(i).arg(j);
	}
	if((mat[0][0] == mat[0][1]) && (mat[0][1] == mat[0][2]) && (mat[0][0] == "O" || mat[0][0] == "X"))
		winner = mat[0][0];
	else if((mat[0][0] == mat[1][1]) && (mat[1][1] == mat[2][2]) && (mat[0][0] == "O" || mat[0][0] == "X"))
		winner = mat[0][0];
	else if((mat[0][0] == mat[1][0]) && (mat[1][0] == mat[2][0]) && (mat[0][0] == "O" || mat[0][0] == "X"))
		winner = mat[0][0];
	else if((mat[0][1] == mat[1][1]) && (mat[1][1] == mat[2][1]) && (mat[0][1] == "O" || mat[0][1] == "X"))
		winner = mat[0][1];
	else if((mat[0][2] == mat[1][2]) && (mat[1][2] == mat[2][2]) && (mat[0][2] == "O" || mat[0][2] == "X"))
		winner = mat[0][2];
	else if((mat[0][2] == mat[1][1]) && (mat[1][1] == mat[0][2]) && (mat[0][2] == "O" || mat[0][2] == "X"))
		winner = mat[0][2];
	else if((mat[1][0] == mat[1][1]) && (mat[1][1] == mat[1][2]) && (mat[1][0] == "O" || mat[1][0] == "X"))
		winner = mat[1][0];
	else if((mat[2][0] == mat[2][1]) && (mat[2][1] == mat[2][2]) && (mat[2][0] == "O" || mat[2][0] == "X"))
		winner = mat[2][0];
	else
		return;
	ui->statusBar->showMessage(" ");
	if((winner == "X" && isserver) || (winner == "O" && !isserver))
		QMessageBox::information(this, "Winner", "You won!!!");
	else if((winner == "X" && !isserver) || (winner == "O" && isserver))
		QMessageBox::critical(this, "Loser", "You lost.");
	moves = 0;
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 3; j++)
			if(ui->tableWidget->item(i, j))
				ui->tableWidget->item(i, j)->~QTableWidgetItem();
	}
	ui->tableWidget->clearContents();
	ui->tableWidget->clear();
	ui->tableWidget->reset();
	ui->playButton->setEnabled(TRUE);
	socket->flush();
	socket->disconnectFromHost();
	socket->close();
	socket->deleteLater();
	if(isserver)
	{
		server->disconnect();
		server->close();
		server->deleteLater();
	}
	ui->gameBox->setEnabled(FALSE);
	ui->connectBox->setEnabled(TRUE);
}
