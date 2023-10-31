#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QTextStream>

#include "DataReport.h"
#include "TrafficAnalysis.h"
#include "PerformanceAnalysis.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->listData = NULL;

    QStringList header;

    header << tr("Fclk\n(MHz)")
           << tr("Analyzed\nPackets")
           << tr("Offered\nTraffic\n(norm)")
           << tr("Accepted\nTraffic\n(norm)")
           << tr("Ideal\nAvg\nLatency\n(cycles)")
           << tr("Avg\nLatency\n(cycles)")
           << tr("Min\nLatency\n(cycles)")
           << tr("Max\nLatency\n(cycles)")
           << tr("Std Dev\nLatency")
           << tr("RT0 Met\nDeadlines\n(%)")
           << tr("RT1 Met\nDeadlines\n(%)")
           << tr("nRT0 Met\nDeadlines\n(%)")
           << tr("nRT1 Met\nDeadlines\n(%)")
           << tr("Offered\nTraffic\n(Mbps/n)")
           << tr("Accepted\nTraffic\n(Mbps/n)")
           << tr("Ideal\nAvg\nLatency\n(ns)")
           << tr("Avg\nLatency\n(ns)")
           << tr("Min\nLatency\n(ns)")
           << tr("Max\nLatency\n(ns)");;

    this->ui->treeView->setHeaderLabels(header);

    for(int i = 0; i < header.size(); i++) {
        this->ui->treeView->headerItem()->setTextAlignment(i,Qt::AlignCenter);
        this->ui->treeView->resizeColumnToContents(i);
    }

    connect(ui->actionExit,SIGNAL(triggered(bool)),this,SLOT(close()));
    connect(ui->actionOpen,SIGNAL(triggered(bool)),this,SLOT(loadDirectory()));
    connect(ui->actionGenerate_Report,SIGNAL(triggered(bool)),this,SLOT(generateCSVReport()));
}

void MainWindow::loadDirectory() {

    workDir = QFileDialog::getExistingDirectory(this,tr("Select the results directory"));

    if( workDir.isNull() || workDir.isEmpty() ) {
        return;
    }

    loadData();


}

void MainWindow::loadData() {

    QString fClkString = workDir.mid( workDir.lastIndexOf("/")+1 );
    fClkString.remove("MHz");

    float fClk = fClkString.toFloat();
    float tClk = (1.0 / fClk) * 1000.0;

    unsigned int xSize = 4;
    unsigned int ySize = 4;
    unsigned int dataWidth = 32;

    float lower = 0; // From 0%
    float upper = 1; // To 100%

    unsigned long channelBW = dataWidth * fClk;

    unsigned int fifoOut = 0;
    unsigned int flowControl = 1; // 0: Handshake | 1: Credit-based

    QString resultDir = workDir + "/Results";
    QDir dir(resultDir);
    if( !dir.exists() ) {
        dir.mkpath(".");
    } else {
        dir.removeRecursively();
        dir.mkpath(".");
    }

    QByteArray dirAnalysis = workDir.toUtf8();
    QByteArray dirResults = resultDir.toUtf8();
    const char* analysis = dirAnalysis.constData();
    const char* results = dirResults.constData();

    TrafficAnalysis* analyzer = new PerformanceAnalysis(xSize,ySize,dataWidth,lower,
                                                    upper,fClk,tClk,channelBW,fifoOut,
                                                    flowControl,analysis,results);

    TrafficAnalysis::StatusAnalysis resultAnalysis = analyzer->makeAnalysis();
    QString dirAnalyzed = fClkString + "MHz";

    switch (resultAnalysis) {
        case TrafficAnalysis::NoInputFile:
            QMessageBox::critical(this,tr("Problem"),tr("There is no input file for analysis (maybe "
                                  "simulation was not run or directory is invalid) in %1").arg(dirAnalyzed));
            return;
        case TrafficAnalysis::NoPacketsDelivered:
            QMessageBox::critical(this,tr("Problem"),tr("Simulation time too short. None packet was "
                                  "delivered in %1").arg(dirAnalyzed));
            return;
        case TrafficAnalysis::NoOutputFile:
            QMessageBox::critical(this,tr("Problem"),tr("Impossible create result file in %1")
                              .arg(dirAnalyzed));
            return;
        default: break;
    }

    delete analyzer;


    // Read the summary of simulation only
    if( this->readReport(resultDir+"/summary") ) {
        QMessageBox::information(this,tr("Information"),tr("Successfully analyzed - %1")
                    .arg(dirAnalyzed));
        this->showData();
    } else {
        QMessageBox::critical(this,tr("Error"),tr("It isn't possible open file from simulation's result"));
    }

}

bool MainWindow::readReport(QString file) {
    QFile* arquivo = new QFile(QString(file));

    if( !arquivo->open(QIODevice::ReadOnly | QIODevice::Text) ) {
        delete arquivo;
        return false;
    } else {
        QTextStream str(&*arquivo);
        QString linha;

        this->clearData();
        QList<DataReport*>* listaDados = new QList<DataReport*>();

        do {
            linha = str.readLine();
            if( linha.isNull() ) {
                break;
            }
            QStringList params = linha.split("\t");
            if(params.size() == 19) {
                DataReport* data = new DataReport;
                data->fClk                  = params.at(0).toFloat();
                data->accNbOfPck            = params.at(1).toULong();
                data->avgRequiredBwNorm     = params.at(2).toFloat();
                data->acceptedTrafficFlits  = params.at(3).toFloat();
                data->idealAvgLatency       = params.at(4).toFloat();
                data->avgLatencyCycles      = params.at(5).toFloat();
                data->stdevLatency          = params.at(6).toFloat();
                data->minLatency            = params.at(7).toFloat();
                data->maxLatency            = params.at(8).toFloat();
                data->avgRequiredBwBps      = params.at(9).toFloat();
                data->acceptedTrafficBps    = params.at(10).toFloat();
                data->idealAvgLatencyNs     = params.at(11).toFloat();
                data->avgLatencyNs          = params.at(12).toFloat();
                data->minLatencyNs          = params.at(13).toFloat();
                data->maxLatencyNs          = params.at(14).toFloat();
                data->metDeadlinesPer0      = params.at(15).toFloat();
                data->metDeadlinesPer1      = params.at(16).toFloat();
                data->metDeadlinesPer2      = params.at(17).toFloat();
                data->metDeadlinesPer3      = params.at(18).toFloat();
                listaDados->append(data);
            }


        } while(!linha.isNull());
        arquivo->close();
        delete arquivo;
        this->listData = listaDados;
        return true;
    }
}

void MainWindow::generateCSVReport() {

    QString filename = QFileDialog::getSaveFileName(this,tr("Save CSV Report"),QString(),tr("CSV Report(*.csv)"));
    if( filename.isNull() || filename.isEmpty() ) {
        return;
    }

    if( listData == NULL ) {
        QMessageBox::information(this,tr("Information"),tr("There is no data loaded!"));
        return;
    }

    QFile file(filename);

    if( !file.open(QIODevice::WriteOnly | QIODevice::Text) ) {
        QMessageBox::critical(this,tr("Error"),tr("It is not possible generate CSV."
                                                 "<br />Please verify write permissions in selected folder."));
        return;
    } else {

        QString csvText;
        QTextStream ts(&csvText);

        QLocale local = QLocale::system();
//        QChar decPoint = local.decimalPoint();
        QString decPoint = local.decimalPoint();
        char separator = ',';
        if( decPoint == ',' ) {
            separator = ';';
        }
        ts << tr("Analyzer CSV Report") << separator
           << separator << separator << separator << separator << separator << separator
           << separator << separator << separator << separator << separator << separator
           << separator << separator << separator << separator << separator << separator
           << separator << separator << "\n";
        // Varrer e escrever CSV
        if( listData != NULL ) {
            // Legenda
            ts << tr("Clock Frequency(MHz)") << separator
               << tr("Analyzed Packets") << separator
               << tr("Offered Traffic(norm)") << separator
               << tr("Accepted Traffic(norm)") << separator
               << tr("Ideal Average Latency(cycles)") << separator
               << tr("Average Latency(cycles)") << separator
               << tr("Standard Deviation Latency") << separator
               << tr("Minimum Latency(cycles)") << separator
               << tr("Maximum Latency(cycles)") << separator
               << tr("Offered Traffic(Mbps/n)") << separator
               << tr("Accepted Traffic(Mbps/n)") << separator
               << tr("Ideal Average Latency(ns)") << separator
               << tr("Average Latency(ns)") << separator
               << tr("Minimun Latency(ns)") << separator
               << tr("Maximum Latency(ns)") << separator
               << tr("RT0 Met Deadlines(%)") << separator
               << tr("RT1 Met Deadlines(%)") << separator
               << tr("nRT0 Met Deadlines(%)") << separator
               << tr("nRT1 Met Deadlines(%)") << "\n";
            for( int z = 0; z < listData->size(); z++ ) {
                // Linha de dado
                DataReport* d = listData->at(z);
                ts << QString::number(d->fClk,'f',0) << separator
                   << QString::number(d->accNbOfPck,'f',0) << separator
                   << QString::number(d->avgRequiredBwNorm,'f',4) << separator
                   << QString::number(d->acceptedTrafficFlits,'f',4) << separator
                   << QString::number(d->idealAvgLatency,'f',0) << separator
                   << QString::number(d->avgLatencyCycles,'f',1) << separator
                   << QString::number(d->stdevLatency,'f',1) << separator
                   << QString::number(d->minLatency,'f',0) << separator
                   << QString::number(d->maxLatency,'f',0) << separator
                   << QString::number(d->avgRequiredBwBps,'f',0) << separator
                   << QString::number(d->acceptedTrafficBps,'f',0) << separator
                   << QString::number(d->idealAvgLatencyNs,'f',1) << separator
                   << QString::number(d->avgLatencyNs,'f',1) << separator
                   << QString::number(d->minLatencyNs,'f',1) << separator
                   << QString::number(d->maxLatencyNs,'f',1) << separator
                   << (d->metDeadlinesPer0 == -1 ? QString():QString::number(d->metDeadlinesPer0)) << separator
                   << (d->metDeadlinesPer1 == -1 ? QString():QString::number(d->metDeadlinesPer1)) << separator
                   << (d->metDeadlinesPer2 == -1 ? QString():QString::number(d->metDeadlinesPer2)) << separator
                   << (d->metDeadlinesPer3 == -1 ? QString():QString::number(d->metDeadlinesPer3)) << "\n";
            }
        }
        if( decPoint == ',' ) {
            csvText.replace(".",","); // Change decimal separator to ','
        }

        QTextStream csvTs(&file);
        csvTs << csvText;
        file.close();
    }
}

void MainWindow::showData() {

    QTreeWidgetItemIterator it(ui->treeView);

    for( ; (*it) ; ) {
        QTreeWidgetItem* item = (*it);
        delete item;
        it++;
    }

    ui->treeView->clear();
    int size = listData->size();
    for( int i = 0; i < size; i++ ) {
        DataReport* dado = listData->at(i);

        QTreeWidgetItem* item = new QTreeWidgetItem(this->ui->treeView);
        item->setData(0,Qt::DisplayRole, QString::number(dado->fClk,'f',0) );
        item->setData(1,Qt::DisplayRole, QString::number(dado->accNbOfPck,'f',0) );
        item->setData(2,Qt::DisplayRole, QString::number(dado->avgRequiredBwNorm,'f',4) );
        item->setData(3,Qt::DisplayRole, QString::number(dado->acceptedTrafficFlits,'f',4) );
        item->setData(4,Qt::DisplayRole, QString::number(dado->idealAvgLatency,'f',0) );
        item->setData(5,Qt::DisplayRole, QString::number(dado->avgLatencyCycles,'f',1) );
        item->setData(6,Qt::DisplayRole, QString::number(dado->minLatency,'f',0) );
        item->setData(7,Qt::DisplayRole, QString::number(dado->maxLatency,'f',0) );
        item->setData(8,Qt::DisplayRole, QString::number(dado->stdevLatency,'f',1) );
        item->setData(9,Qt::DisplayRole, (dado->metDeadlinesPer0 == -1 ? QString():QString::number(dado->metDeadlinesPer0,'g',4)) );
        item->setData(10,Qt::DisplayRole, (dado->metDeadlinesPer1 == -1 ? QString():QString::number(dado->metDeadlinesPer1,'g',4)) );
        item->setData(11,Qt::DisplayRole, (dado->metDeadlinesPer2 == -1 ? QString():QString::number(dado->metDeadlinesPer2,'g',4)) );
        item->setData(12,Qt::DisplayRole, (dado->metDeadlinesPer3 == -1 ? QString():QString::number(dado->metDeadlinesPer3,'g',4)) );
        item->setData(13,Qt::DisplayRole, QString::number(dado->avgRequiredBwBps,'f',1) );
        item->setData(14,Qt::DisplayRole, QString::number(dado->acceptedTrafficBps,'f',1) );
        item->setData(15,Qt::DisplayRole, QString::number(dado->idealAvgLatencyNs,'f',1) );
        item->setData(16,Qt::DisplayRole, QString::number(dado->avgLatencyNs,'f',1) );
        item->setData(17,Qt::DisplayRole, QString::number(dado->minLatencyNs,'f',1) );
        item->setData(18,Qt::DisplayRole, QString::number(dado->maxLatencyNs,'f',1) );
        for( int x = 0; x < ui->treeView->columnCount(); x++ ) {
            item->setTextAlignment(x,Qt::AlignCenter);
        }
    }

    for(int i = 0; i < ui->treeView->columnCount(); i++) {
        this->ui->treeView->resizeColumnToContents(i);
    }

}

void MainWindow::clearData() {

    if(listData != NULL) {
        for( int z = 0; z < listData->size(); z++ ) {
            DataReport* d = listData->at(z);
            delete d;
        }
        listData->clear();
        delete listData;
        listData = NULL;
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}
