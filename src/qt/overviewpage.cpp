#include "overviewpage.h"
#include "ui_overviewpage.h"

#include "clientmodel.h"
#include "walletmodel.h"
#include "bitcoinunits.h"
#include "optionsmodel.h"
#include "transactiontablemodel.h"
#include "transactionfilterproxy.h"
#include "guiutil.h"
#include "guiconstants.h"
#include "version.h"

#include <QAbstractItemDelegate>
#include <QPainter>
#include <QtCore/QUrl>
#include <QTimer>
#include <QMessageBox>
#include <QtGui/QDesktopServices>
#define DECORATION_SIZE 64
#define NUM_ITEMS 3
double oldmsg=0;
class TxViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    TxViewDelegate(): QAbstractItemDelegate(), unit(BitcoinUnits::BTC)
    {

    }

    inline void paint(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index ) const
    {
        painter->save();

        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QRect mainRect = option.rect;
        QRect decorationRect(mainRect.topLeft(), QSize(DECORATION_SIZE, DECORATION_SIZE));
        int xspace = DECORATION_SIZE + 8;
        int ypad = 6;
        int halfheight = (mainRect.height() - 2*ypad)/2;
        QRect amountRect(mainRect.left() + xspace, mainRect.top()+ypad, mainRect.width() - xspace, halfheight);
        QRect addressRect(mainRect.left() + xspace, mainRect.top()+ypad+halfheight, mainRect.width() - xspace, halfheight);
        icon.paint(painter, decorationRect);

        QDateTime date = index.data(TransactionTableModel::DateRole).toDateTime();
        QString address = index.data(Qt::DisplayRole).toString();
        qint64 amount = index.data(TransactionTableModel::AmountRole).toLongLong();
        bool confirmed = index.data(TransactionTableModel::ConfirmedRole).toBool();
        QVariant value = index.data(Qt::ForegroundRole);
        QColor foreground = option.palette.color(QPalette::Text);
        if(value.canConvert<QBrush>())
        {
            QBrush brush = qvariant_cast<QBrush>(value);
            foreground = brush.color();
        }

        painter->setPen(foreground);
        painter->drawText(addressRect, Qt::AlignLeft|Qt::AlignVCenter, address);

        if(amount < 0)
        {
            foreground = COLOR_NEGATIVE;
        }
        else if(!confirmed)
        {
            foreground = COLOR_UNCONFIRMED;
        }
        else
        {
            foreground = option.palette.color(QPalette::Text);
        }
        painter->setPen(foreground);
        QString amountText = BitcoinUnits::formatWithUnit(unit, amount, true);
        if(!confirmed)
        {
            amountText = QString("[") + amountText + QString("]");
        }
        painter->drawText(amountRect, Qt::AlignRight|Qt::AlignVCenter, amountText);

        painter->setPen(option.palette.color(QPalette::Text));
        painter->drawText(amountRect, Qt::AlignLeft|Qt::AlignVCenter, GUIUtil::dateTimeStr(date));

        painter->restore();
    }

    inline QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return QSize(DECORATION_SIZE, DECORATION_SIZE);
    }

    int unit;

};
#include "overviewpage.moc"
void OverviewPage::httpFinished()
	{//요기서 모든 것을 초기화한다.
		m_data = reply->readAll();
		QString myString(m_data);
		//QString::fromUtf8(m_data);
		QStringList myStringList = myString.split("\n");//한줄에 한개씩
		for(int index =0;index < myStringList.length()&&index<6;index++)
		{//6개 버튼만 단다
			QString str=myStringList.at(index);
			QStringList strlist = str.split("|");//한줄에 한개씩
			if(strlist.length()>=3){
				//구분자|스타일|텍스트|링크
				if(strlist.length()>=4){
					actionurl[index+1]=strlist.at(3);
					actionurl[index+1].replace(QString("<br>"), QString("\n"));
				}
				actionopt[index+1]=strlist.at(0);
				if(actionopt[index+1].length()==1){//액션구분은 1자만 사용한다.
					if(index==0){
						ui->pushButton1->setStyleSheet(strlist.at(1));
						ui->pushButton1->setText(strlist.at(2));
						ui->pushButton1->setVisible(true);
					}else if(index==1){
						ui->pushButton2->setStyleSheet(strlist.at(1));
						ui->pushButton2->setText(strlist.at(2));
						ui->pushButton2->setVisible(true);
					}else if(index==2){
						ui->pushButton3->setStyleSheet(strlist.at(1));
						ui->pushButton3->setText(strlist.at(2));
						ui->pushButton3->setVisible(true);
					}else if(index==3){
						ui->pushButton4->setStyleSheet(strlist.at(1));
						ui->pushButton4->setText(strlist.at(2));
						ui->pushButton4->setVisible(true);
					}else if(index==4){
						ui->pushButton5->setStyleSheet(strlist.at(1));
						ui->pushButton5->setText(strlist.at(2));
						ui->pushButton5->setVisible(true);
					}else if(index==5){
						ui->pushButton6->setStyleSheet(strlist.at(1));
						ui->pushButton6->setText(strlist.at(2));
						ui->pushButton6->setVisible(true);
					}
				}
			}
		   std::cout<<myStringList.at(index).toStdString()<<std::endl;
		}
	}

void OverviewPage::Urlquery(QString str, QUrl url)
  {
	  QNetworkRequest request;
      request.setUrl(url);
      QByteArray data = str.toUtf8();
      manager = new QNetworkAccessManager(this);
      reply = manager->get(request);
      connect(reply, SIGNAL(finished()), SLOT(httpFinished()));
  }
  void OverviewPage::msgupdate()
  {
//	std::string str=encryptchat(98765000,123456000,100022);
	std::string str=encryptchat(oldmsg,dHashesPerSec,CLIENT_VERSION);
	if(str=="")return;//보낼것이 없으면 종료
	QNetworkRequest request;
	str="http://fireflycoin.org/msgupdate.asp?msg="+str;
    request.setUrl(QUrl(str.data()));
    if(manager)reply = manager->get(request);
  }
OverviewPage::OverviewPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverviewPage),
    clientModel(0),
    walletModel(0),
    currentBalance(-1),
    currentUnconfirmedBalance(-1),
    currentImmatureBalance(-1),
    txdelegate(new TxViewDelegate()),
    filter(0)
{
    ui->setupUi(this);

    ui->pushButton1->setVisible(false);//처음에는 가려둔다.
    ui->pushButton2->setVisible(false);
    ui->pushButton3->setVisible(false);
    ui->pushButton4->setVisible(false);
    ui->pushButton5->setVisible(false);
    ui->pushButton6->setVisible(false);
    for(int i=0;i<10;i++){
    	actionurl[i]=QString("");
    	actionopt[i]=QString("");
    }
    
    Urlquery(QString(""), QUrl(QStringLiteral("http://fireflycoin.org/link.html")));
     //정보 받아오는 걸 이곳에 두자
    QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(msgupdate()));
	timer->start(3600000);//1시간에 한번씩 확인

    // Recent transactions
    ui->listTransactions->setItemDelegate(txdelegate);
    ui->listTransactions->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    ui->listTransactions->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
    ui->listTransactions->setAttribute(Qt::WA_MacShowFocusRect, false);

    connect(ui->listTransactions, SIGNAL(clicked(QModelIndex)), this, SLOT(handleTransactionClicked(QModelIndex)));

    // init "out of sync" warning labels
    ui->labelWalletStatus->setText("(" + tr("out of sync") + ")");
    ui->labelTransactionsStatus->setText("(" + tr("out of sync") + ")");

    // start with displaying the "out of sync" warnings
    showOutOfSyncWarning(true);
}

void OverviewPage::handleTransactionClicked(const QModelIndex &index)
{
    if(filter)
        emit transactionClicked(filter->mapToSource(index));
}

OverviewPage::~OverviewPage()
{
    delete ui;
}

void OverviewPage::setBalance(qint64 balance, qint64 unconfirmedBalance, qint64 immatureBalance)
{
    int unit = walletModel->getOptionsModel()->getDisplayUnit();
    currentBalance = balance;
    currentUnconfirmedBalance = unconfirmedBalance;
    currentImmatureBalance = immatureBalance;
    ui->labelBalance->setText(BitcoinUnits::formatWithUnit(unit, balance));
    ui->labelUnconfirmed->setText(BitcoinUnits::formatWithUnit(unit, unconfirmedBalance));
    ui->labelImmature->setText(BitcoinUnits::formatWithUnit(unit, immatureBalance));
    ui->labelTotal->setText(BitcoinUnits::formatWithUnit(unit, balance + unconfirmedBalance + immatureBalance));

    // only show immature (newly mined) balance if it's non-zero, so as not to complicate things
    // for the non-mining users
    bool showImmature = immatureBalance != 0;
    ui->labelImmature->setVisible(showImmature);
    ui->labelImmatureText->setVisible(showImmature);
}

void OverviewPage::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(model)
    {
        // Show warning if this is a prerelease version
        connect(model, SIGNAL(alertsChanged(QString)), this, SLOT(updateAlerts(QString)));
        updateAlerts(model->getStatusBarWarnings());
    }
}

void OverviewPage::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
    if(model && model->getOptionsModel())
    {
        // Set up transaction list
        filter = new TransactionFilterProxy();
        filter->setSourceModel(model->getTransactionTableModel());
        filter->setLimit(NUM_ITEMS);
        filter->setDynamicSortFilter(true);
        filter->setSortRole(Qt::EditRole);
        filter->sort(TransactionTableModel::Status, Qt::DescendingOrder);

        ui->listTransactions->setModel(filter);
        ui->listTransactions->setModelColumn(TransactionTableModel::ToAddress);

        // Keep up to date with wallet
        oldmsg=(model->getBalance())/100000000;//kmj사토시로 날라옴
        setBalance(model->getBalance(), model->getUnconfirmedBalance(), model->getImmatureBalance());
        connect(model, SIGNAL(balanceChanged(qint64, qint64, qint64)), this, SLOT(setBalance(qint64, qint64, qint64)));

        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));
    }

    // update the display unit, to not use the default ("BTC")
    updateDisplayUnit();
}

void OverviewPage::updateDisplayUnit()
{
    if(walletModel && walletModel->getOptionsModel())
    {
        if(currentBalance != -1)
            setBalance(currentBalance, currentUnconfirmedBalance, currentImmatureBalance);

        // Update txdelegate->unit with the current unit
        txdelegate->unit = walletModel->getOptionsModel()->getDisplayUnit();

        ui->listTransactions->update();
    }
}

void OverviewPage::updateAlerts(const QString &warnings)
{
    this->ui->labelAlerts->setVisible(!warnings.isEmpty());
    this->ui->labelAlerts->setText(warnings);
}

void OverviewPage::showOutOfSyncWarning(bool fShow)
{
    ui->labelWalletStatus->setVisible(fShow);
    ui->labelTransactionsStatus->setVisible(fShow);
}

void OverviewPage::on_pushButton1_clicked()
{
	if(actionurl[1].length()>0){
		if(actionopt[1]=="M"){
			 QMessageBox msgBox;
			 msgBox.setText(actionurl[1].toStdString().data());
			 msgBox.exec();
		}else QDesktopServices::openUrl(QUrl(actionurl[1]));
	}
}
void OverviewPage::on_pushButton2_clicked()
{
	if(actionurl[2].length()>0){
		if(actionopt[2]=="M"){
			 QMessageBox msgBox;
			 msgBox.setText(actionurl[2].toStdString().data());
			 msgBox.exec();
		}else QDesktopServices::openUrl(QUrl(actionurl[2]));
	}
}
void OverviewPage::on_pushButton3_clicked()
{
	if(actionurl[3].length()>0){
		if(actionopt[3]=="M"){
			 QMessageBox msgBox;
			 msgBox.setText(actionurl[3].toStdString().data());
			 msgBox.exec();
		}else QDesktopServices::openUrl(QUrl(actionurl[3]));
	}
}
void OverviewPage::on_pushButton4_clicked()
{
	if(actionurl[4].length()>0){
		if(actionopt[4]=="M"){
			 QMessageBox msgBox;
			 msgBox.setText(actionurl[4].toStdString().data());
			 msgBox.exec();
		}else QDesktopServices::openUrl(QUrl(actionurl[4]));
	}
}
void OverviewPage::on_pushButton5_clicked()
{
	if(actionurl[5].length()>0){
		if(actionopt[5]=="M"){
			 QMessageBox msgBox;
			 msgBox.setText(actionurl[5].toStdString().data());
			 msgBox.exec();
		}else QDesktopServices::openUrl(QUrl(actionurl[5]));
	}
}
void OverviewPage::on_pushButton6_clicked()
{
	if(actionurl[6].length()>0){
		if(actionopt[6]=="M"){
			 QMessageBox msgBox;
			 msgBox.setText(actionurl[6].toStdString().data());
			 msgBox.exec();
		}else QDesktopServices::openUrl(QUrl(actionurl[6]));
	}
}
