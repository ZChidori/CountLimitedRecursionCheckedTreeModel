#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "countlimitedrecursioncheckedmodel.h"
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	CountLimitedRecursionCheckedModel *clrcm=new CountLimitedRecursionCheckedModel(ui->treeView);
	for(int i=0;i<5;)
	{
		QStandardItem *qsi1=new QStandardItem("1层"+QString::number(i++));
		qsi1->setAutoTristate(true);
		qsi1->setCheckState(Qt::Unchecked);
		qsi1->setCheckable(true);
		clrcm->item(0)->appendRow(qsi1);
		for(int j=0;j<5;)
		{
			QStandardItem *qsi2=new QStandardItem("2层"+QString::number(j++));
			qsi2->setAutoTristate(true);
			qsi2->setCheckState(Qt::Unchecked);
			qsi2->setCheckable(true);
			qsi1->appendRow(qsi2);
			for(int k=0;k<5;)
			{
				QStandardItem *qsi3=new QStandardItem("3层"+QString::number(k++));
				qsi3->setAutoTristate(true);
				qsi3->setCheckState(Qt::Unchecked);
				qsi3->setCheckable(true);
				qsi2->appendRow(qsi3);
			}
		}
	}
	clrcm->setLevelLimit(1,2);
	clrcm->setLevelLimit(2,2);
	clrcm->setLevelLimit(3,2);
	ui->treeView->setModel(clrcm);
	ui->treeView->expandAll();
	connect(clrcm,&CountLimitedRecursionCheckedModel::signalLevelLimitExceeded,this,&MainWindow::slotLimitError);
}

void MainWindow::slotLimitError(std::vector<LevelState> levelVector)
{
	QString s;
	for(int i=0,si=levelVector.size();i<si;i++)
		s+="第"+QString::number(levelVector[i].level)+"层超出限制，可选中"+QString::number(levelVector[i].max)+"，已选中"+QString::number(levelVector[i].checked)+'\n';
	QMessageBox::information(this,"提示",s);
}

MainWindow::~MainWindow()
{
	delete ui;
}


void MainWindow::on_pushButton_clicked()
{
	qDebug()<<((CountLimitedRecursionCheckedModel *)ui->treeView->model())->getCheckedCount(4);
}

