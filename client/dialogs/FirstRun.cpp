/*
 * QDigiDoc4
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include "FirstRun.h"
#include "ui_FirstRun.h"
#include "Styles.h"
#include "common/Settings.h"

#include <QDebug>
#include <QKeyEvent>
#include <QLineEdit>


//bool ArrowKeyFilter::eventFilter(QObject* obj, QEvent* event)
//{
//	if (event->type()==QEvent::KeyPress)
//	{
//		QKeyEvent* key = static_cast<QKeyEvent*>(event);
//		if ( key->key()==Qt::Key_Left || key->key() == Qt::Key_Right )
//		{
//			FirstRun *dlg = qobject_cast<FirstRun*>( obj );
//			if( dlg )
//			{
//				dlg->navigate( key->key()==Qt::Key_Right );
//				return true;
//			}
//		}
//	}

//	return QObject::eventFilter(obj, event);
//}



FirstRun::FirstRun(QWidget *parent) :
  QDialog(parent)
, ui(new Ui::FirstRun)
{
	ui->setupUi(this);
	setWindowFlags( Qt::Dialog | Qt::FramelessWindowHint );
	setWindowModality( Qt::ApplicationModal );

//	installEventFilter( new ArrowKeyFilter );


	QFont titleFont = Styles::font(Styles::Regular, 18);
	QFont subTitleFont = Styles::font(Styles::Regular, 16);
	QFont regularFont = Styles::font( Styles::Regular, 14 );
	QFont buttonFont = Styles::font( Styles::Condensed, 14 );
	QFont smallFont = Styles::font(Styles::Condensed, 12);

	titleFont.setWeight( QFont::DemiBold );
	subTitleFont.setWeight( QFont::DemiBold );


	// pageWelcome
	ui->welcomeTitle->setFont(titleFont);
	ui->welcomeDescription->setFont(regularFont);
	ui->welcomeSelectLang->setFont(smallFont);
	ui->welcomeLanguage->setFont(Styles::font(Styles::Regular, 18));
	ui->welcomeContinue->setFont(buttonFont);

	ui->welcomeLanguage->addItem("Eesti keel");
	ui->welcomeLanguage->addItem("English");
	ui->welcomeLanguage->addItem("Русский язык");

	if(Settings::language() == "en")
	{
		ui->welcomeLanguage->setCurrentIndex(1);
	}
	else if(Settings::language() == "ru")
	{
		ui->welcomeLanguage->setCurrentIndex(2);
	}
	else
	{
		ui->welcomeLanguage->setCurrentIndex(0);
	}

	connect( ui->welcomeLanguage, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
		[this](int index)
		{
			switch(index)
			{
			case 1:
				emit langChanged("en");
				break;
			case 2:
				emit langChanged("ru");
				break;
			default:
				emit langChanged("et");
				break;
			}

			ui->retranslateUi(this);
		}
			);

	connect(ui->welcomeContinue, &QPushButton::clicked, this, [this](){ ui->stackedWidget->setCurrentIndex(1); } );

	// PageIntro
	ui->introTitle->setFont(titleFont);
	ui->introSigningTitle->setFont(subTitleFont);
	ui->introSigningBrief->setFont(regularFont);
	ui->introEncryptingTitle->setFont(subTitleFont);
	ui->introEncryptingBrief->setFont(regularFont);
	ui->introMyIDsTitle->setFont(subTitleFont);
	ui->introMyIDsBrief->setFont(regularFont);

	ui->viewSigning->setFont(buttonFont);
	ui->viewEncryption->setFont(buttonFont);
	ui->viewEid->setFont(buttonFont);
	ui->introSkip->setFont(smallFont);

	connect(ui->viewSigning, &QPushButton::clicked, this, [this](){ ui->stackedWidget->setCurrentIndex(2); } );
	connect(ui->viewEncryption, &QPushButton::clicked, this, [this](){ ui->stackedWidget->setCurrentIndex(3); } );
	connect(ui->viewEid, &QPushButton::clicked, this, [this](){ ui->stackedWidget->setCurrentIndex(4); } );
	connect(ui->introSkip, &QPushButton::clicked, this, [this]() { emit close(); } );
}

FirstRun::~FirstRun()
{
	delete ui;
}


//void FirstRun::toPage( View toPage )
//{
//	page = toPage;
//	if( toPage == Language )
//	{
//		ui->lang->show();
//		ui->continue_2->show();

//		ui->viewSigning->hide();
//		ui->viewEncryption->hide();
//		ui->viewEid->hide();
//		ui->next->hide();
//		ui->skip->hide();
//		ui->gotoSigning->hide();
//		ui->gotoEncryption->hide();
//		ui->gotoEid->hide();
	
//		setStyleSheet("image: url(:/images/FirstRun1.png);");
//	}
//	else
//	{
//		ui->lang->hide();
//		ui->continue_2->hide();

//		if( toPage == Intro)
//		{
//			ui->viewSigning->show();
//			ui->viewEncryption->show();
//			ui->viewEid->show();
//			ui->skip->show();

//			ui->next->hide();
//			ui->gotoSigning->hide();
//			ui->gotoEncryption->hide();
//			ui->gotoEid->hide();
//			setStyleSheet("image: url(:/images/FirstRun2.png);");
//		}
//		else
//		{
//			ui->viewSigning->hide();
//			ui->viewEncryption->hide();
//			ui->viewEid->hide();
//			ui->next->show();
		
//			ui->gotoSigning->show();
//			ui->gotoEncryption->show();
//			ui->gotoEid->show();
//			showDetails();
//		}
//	}
//}

//void FirstRun::showDetails()
//{
//	QString normal = "border: none; image: url(:/images/icon_dot.png);";
//	QString active = "border: none; image: url(:/images/icon_dot_active.png);";

//	ui->gotoSigning->setStyleSheet(page == Signing ? active : normal);
//	ui->gotoEncryption->setStyleSheet(page == Encryption ? active : normal);
//	ui->gotoEid->setStyleSheet(page == MyEid ? active : normal);


//	switch (page) {
//	case Signing:
//		setStyleSheet("image: url(:/images/FirstRunSigning.png);");
//		ui->next->setText(tr("VIEW NEXT"));
//		ui->skip->show();
//		break;
//	case Encryption:
//		setStyleSheet("image: url(:/images/FirstRunEncrypt.png);");
//		ui->next->setText(tr("VIEW NEXT"));
//		ui->skip->show();
//		break;
//	default:
//		setStyleSheet("image: url(:/images/FirstRunMyEID.png);");
//		ui->next->setText(tr("ENTER THE APPLICATION"));
//		ui->skip->hide();
//		break;
//	}
//}


//void FirstRun::navigate( bool forward )
//{
//	if( forward )
//	{
//		if( page == MyEid )
//		{
//			emit close();
//			return;
//		}
//		toPage( static_cast<View>(page + 1) );
//	}
//	else if( page != Language )
//	{
//		toPage( static_cast<View>(page - 1) );
//	}
//}
