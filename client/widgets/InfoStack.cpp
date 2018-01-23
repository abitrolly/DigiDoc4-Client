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

#include "InfoStack.h"
#include "ui_InfoStack.h"

#include "QCardInfo.h"
#include "QSmartCard.h"
#include "Styles.h"
#include "effects/HoverFilter.h"

#include <common/SslCertificate.h>

#include <QSvgWidget>
#include <QtCore/QTextStream>

InfoStack::InfoStack( QWidget *parent )
: StyledWidget( parent )
, ui( new Ui::InfoStack )
, sealWidget(nullptr)
, appletVersion()
, certType(0)
, certIsValid(false)
, citizenshipText()
, expireDate()
, givenNamesText()
, personalCodeText()
, serialNumberText()
, surnameText()
, pictureText("DOWNLOAD")
{
	ui->setupUi( this );

	ui->btnPicture->setFont( Styles::font( Styles::Condensed, 12 ) );
	connect( ui->btnPicture, &QPushButton::clicked, this, [this]() { emit photoClicked( ui->photo->pixmap() ); } );
	
	QFont labelFont = Styles::font(Styles::Condensed, 11);
	ui->labelGivenNames->setFont(labelFont);
	ui->labelSurname->setFont(labelFont);
	ui->labelPersonalCode->setFont(labelFont);
	ui->labelCitizenship->setFont(labelFont);
	ui->labelSerialNumber->setFont(labelFont);

	QFont font = Styles::font(Styles::Regular, 16);
	font.setWeight(QFont::DemiBold);
	ui->valueGivenNames->setFont(font);
	ui->valueSurname->setFont(font);
	ui->valuePersonalCode->setFont(font);
	ui->valueCitizenship->setFont(font);
	ui->valueSerialNumber->setFont(font);
	ui->valueExpiryDate->setFont( Styles::font( Styles::Regular, 16 ) );

	HoverFilter *filter = new HoverFilter(ui->photo, [this](int eventType){ focusEvent(eventType); }, this);
	ui->photo->installEventFilter(filter);
}

InfoStack::~InfoStack()
{
	delete ui;
}

void InfoStack::clearPicture()
{
	ui->photo->clear();
}

void InfoStack::clearData()
{
	ui->valueGivenNames->setText( "" );
	ui->valueSurname->setText( "" );
	ui->valuePersonalCode->setText( "" );
	ui->valueCitizenship->setText( "" );
	ui->valueSerialNumber->setText( "" );
	ui->valueExpiryDate->setText( "" );
	pictureText = "DOWNLOAD";
	ui->btnPicture->setText(tr(qPrintable(pictureText)));
	ui->btnPicture->show();
}

void InfoStack::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
		update();
	}

	QWidget::changeEvent(event);
}


void InfoStack::focusEvent(int eventType)
{
	if(!ui->photo->pixmap())
		return;

	if(eventType == QEvent::Enter)
	{
		ui->btnPicture->show();
	}
	else
	{
		// Ignore multiple enter-leave events sent when moving over button
		auto boundingRect = QRect(ui->photo->mapToGlobal(ui->photo->geometry().topLeft()),
								  ui->photo->mapToGlobal(ui->photo->geometry().bottomRight()));
		if(!boundingRect.contains(QCursor::pos()))
			ui->btnPicture->hide();
	}
}

void InfoStack::showPicture( const QPixmap &pixmap )
{
	ui->photo->setProperty( "PICTURE", pixmap );
	ui->photo->setPixmap( pixmap.scaled( 120, 150, Qt::IgnoreAspectRatio, Qt::SmoothTransformation ) );
	pictureText = "SAVE THE PICTURE";
	ui->btnPicture->setText(tr(qPrintable(pictureText)));
	ui->btnPicture->hide();
}

void InfoStack::update()
{
	QString text;
	QTextStream st( &text );

	if(certType & SslCertificate::EstEidType)
	{
		if(certIsValid)
			st << "<span style='color: #37a447'>" << tr("Valid") << "</span>" << tr("until") << expireDate;
		else
			st << "<span style='color: #e80303;'>" << tr("Expired") << "</span>";
	}
	else if( !(certType & SslCertificate::TempelType))
	{
		st << tr("You're using Digital identity card");
	}

	ui->valueGivenNames->setText(givenNamesText);
	ui->valueSurname->setText(surnameText);
	ui->valuePersonalCode->setText(personalCodeText);
	ui->valueCitizenship->setText(citizenshipText);
	ui->valueExpiryDate->setText(text);
	ui->valueSerialNumber->setText(serialNumberText);
	ui->btnPicture->setText(tr(qPrintable(pictureText)));

	if(certType & SslCertificate::TempelType)
	{
		ui->labelGivenNames->setText(tr("NAME"));
		ui->labelSurname->setText(QString());
		ui->labelPersonalCode->setText(tr("SERIAL"));
		ui->labelCitizenship->setText(tr("COUNTRY"));
		ui->labelSerialNumber->setText(tr("DEVICE"));
		ui->valueSerialNumber->setMinimumWidth(300);
		ui->valueSerialNumber->setMaximumWidth(300);

		clearPicture();
		QSvgWidget *seal = new QSvgWidget(ui->photo);
		seal->load(QString(":/images/icon_digitempel.svg"));
		seal->resize(100, 100);
		seal->move(10, 25);
		seal->setStyleSheet("border: none;");
		seal->show();
		sealWidget = seal;
		ui->btnPicture->hide();
	}
	else
	{
		ui->labelGivenNames->setText(tr("Given names"));
		ui->labelSurname->setText(tr("Surname"));
		ui->labelPersonalCode->setText(tr("Personal code"));
		ui->labelCitizenship->setText(tr("Citizenship"));
		ui->labelSerialNumber->setText(tr("Document"));
		ui->valueSerialNumber->setMinimumWidth(100);
		ui->valueSerialNumber->setMaximumWidth(100);

		if(sealWidget)
		{
			sealWidget->hide();
			sealWidget->close();
			delete sealWidget;
			sealWidget = nullptr;
		}
		ui->btnPicture->show();
	}
}

void InfoStack::update(const QSmartCardData &t)
{
	QStringList firstName = QStringList()
		<< t.data( QSmartCardData::FirstName1 ).toString()
		<< t.data( QSmartCardData::FirstName2 ).toString();
	firstName.removeAll( "" );

	certType = t.authCert().type();
	certIsValid = t.isValid();
	expireDate = DateTime(t.data(QSmartCardData::Expiry).toDateTime()).formatDate( "dd.MM.yyyy" );
	givenNamesText = firstName.join(" ");
	surnameText = t.data(QSmartCardData::SurName).toString();
	personalCodeText = t.data(QSmartCardData::Id).toString();
	citizenshipText = t.data(QSmartCardData::Citizen).toString();
	serialNumberText = t.data(QSmartCardData::DocumentId).toString();
	if(!serialNumberText.isEmpty())
		serialNumberText += "  |" ;

	update();
}

void InfoStack::update(const QCardInfo &cardInfo)
{
	certType = cardInfo.type;
	certIsValid = true;
	expireDate = QString();
	givenNamesText = cardInfo.fullName;
	surnameText = QString();
	personalCodeText = cardInfo.id;
	citizenshipText = cardInfo.country;
	serialNumberText = (certType & SslCertificate::TempelType) ?
		tr("You're using e-Seal") : QString();

	update();
}
