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

#include "QSigner.h"
#include "QCardLock.h"

#include "Application.h"

#ifdef Q_OS_WIN
#include "QCSP.h"
#include "QCNG.h"
#else
class QWin;
#endif
#include "QPKCS11.h"
#include <common/TokenData.h>

#include <digidocpp/crypto/X509Cert.h>

#include <QtCore/QFile>
#include <QtCore/QEventLoop>
#include <QtCore/QStringList>
#include <QtCore/QSysInfo>
#include <QDebug>
#include <QtNetwork/QSslKey>

#include <openssl/obj_mac.h>

#if QT_VERSION < 0x050700
template <class T>
constexpr typename std::add_const<T>::type& qAsConst(T& t) noexcept
{
	return t;
}
#endif

class QSignerPrivate
{
public:
	QSigner::ApiType api = QSigner::PKCS11;
	QWin			*win = nullptr;
	QPKCS11Stack	*pkcs11 = nullptr;
	TokenData		auth, sign;
};

using namespace digidoc;

QSigner::QSigner( ApiType api, QObject *parent )
:	d( new QSignerPrivate )
{
	d->api = api;
	d->auth.setCard( "loading" );
	d->sign.setCard( "loading" );
	connect(this, &QSigner::error, this, &QSigner::showWarning);
}

QSigner::~QSigner()
{
	delete d;
}

X509Cert QSigner::cert() const
{
	if( d->sign.cert().isNull() )
		throw Exception( __FILE__, __LINE__, QSigner::tr("Sign certificate is not selected").toUtf8().constData() );
	QByteArray der = d->sign.cert().toDer();
	return X509Cert((const unsigned char*)der.constData(), size_t(der.size()), X509Cert::Der);
}

QSigner::ErrorCode QSigner::decrypt(const QByteArray &in, QByteArray &out, const QString &digest, int keySize,
	const QByteArray &algorithmID, const QByteArray &partyUInfo, const QByteArray &partyVInfo)
{
	if(!QCardLock::instance().exclusiveTryLock())
	{
		Q_EMIT error( tr("Signing/decrypting is already in progress another window.") );
		return DecryptFailed;
	}

	if( !d->auth.cards().contains( d->auth.card() ) || d->auth.cert().isNull() )
	{
		Q_EMIT error( tr("Authentication certificate is not selected.") );
		QCardLock::instance().exclusiveUnlock();
		return DecryptFailed;
	}

	if( d->pkcs11 )
	{
		QPKCS11::PinStatus status = d->pkcs11->login( d->auth );
		switch( status )
		{
		case QPKCS11::PinOK: break;
		case QPKCS11::PinCanceled:
			QCardLock::instance().exclusiveUnlock();
			return PinCanceled;
		case QPKCS11::PinIncorrect:
			QCardLock::instance().exclusiveUnlock();
			reloadauth();
			if( !(d->auth.flags() & TokenData::PinLocked) )
			{
				Q_EMIT error( QPKCS11::errorString( status ) );
				return PinIncorrect;
			}
			// else pin locked, fall through
		case QPKCS11::PinLocked:
			QCardLock::instance().exclusiveUnlock();
			Q_EMIT error( QPKCS11::errorString( status ) );
			return PinLocked;
		default:
			QCardLock::instance().exclusiveUnlock();
			Q_EMIT error( tr("Failed to login token") + " " + QPKCS11::errorString( status ) );
			return DecryptFailed;
		}
		if(d->auth.cert().publicKey().algorithm() == QSsl::Rsa)
			out = d->pkcs11->decrypt(in);
		else
			out = d->pkcs11->deriveConcatKDF(in, digest, keySize, algorithmID, partyUInfo, partyVInfo);
		d->pkcs11->logout();
	}
#ifdef Q_OS_WIN
	else if(d->win)
	{
		d->win->selectCert(d->auth.cert());
		if(d->auth.cert().publicKey().algorithm() == QSsl::Rsa)
			out = d->win->decrypt(in);
		else
			out = d->win->deriveConcatKDF(in, digest, keySize, algorithmID, partyUInfo, partyVInfo);
		if(d->win->lastError() == QWin::PinCanceled)
		{
			QCardLock::instance().exclusiveUnlock();
			return PinCanceled;
		}
	}
#endif

	if( out.isEmpty() )
		Q_EMIT error( tr("Failed to decrypt document") );
	QCardLock::instance().exclusiveUnlock();
	reloadauth();
	return !out.isEmpty() ? DecryptOK : DecryptFailed;
}

void QSigner::init()
{
	d->auth.clear();
	d->auth.setCard( "loading" );
	d->sign.clear();
	d->sign.setCard( "loading" );

	switch( d->api )
	{
#ifdef Q_OS_WIN
	case CAPI: d->win = new QCSP( this ); break;
	case CNG: d->win = new QCNG( this ); break;
#endif
	default: d->pkcs11 = new QPKCS11Stack( this ); break;
	}

	QString driver = qApp->confValue( Application::PKCS11Module ).toString();
	if( d->pkcs11 && !d->pkcs11->isLoaded() && !d->pkcs11->load( driver ) )
	{
		Q_EMIT error( tr("Failed to load PKCS#11 module") + "\n" + driver );
		return;
	}


}

void QSigner::reloadauth() const
{
	QEventLoop e;
	QObject::connect(this, &QSigner::authDataChanged, &e, &QEventLoop::quit);
	{
		QCardLocker locker;
		d->auth.setCert( QSslCertificate() );
	}
	e.exec();
}

void QSigner::reloadsign() const
{
	QEventLoop e;
	QObject::connect(this, &QSigner::signDataChanged, &e, &QEventLoop::quit);
	{
		QCardLocker locker;
		d->sign.setCert( QSslCertificate() );
	}
	e.exec();
}

void QSigner::select(const TokenData &authToken, const TokenData &signToken)
{
#ifdef Q_OS_WIN
	d->auth = authToken;
	d->auth.setCert(QSslCertificate());
	d->sign = signToken;
	d->sign.setCert(QSslCertificate());
	update();
#else
	Q_EMIT signDataChanged(d->auth = authToken);
	Q_EMIT signDataChanged(d->sign = signToken);
#endif
}

void QSigner::selectAuthCard( const QString &card )
{
	TokenData t = d->auth;
	t.setCard( card );
	t.setCert( QSslCertificate() );
	Q_EMIT signDataChanged(d->auth = t);
}

void QSigner::selectSignCard( const QString &card )
{
	TokenData t = d->sign;
	t.setCard( card );
	t.setCert( QSslCertificate() );
	Q_EMIT signDataChanged(d->sign = t);
}

void QSigner::showWarning( const QString &msg )
{ qApp->showWarning( msg ); }

std::vector<unsigned char> QSigner::sign(const std::string &method, const std::vector<unsigned char> &digest ) const
{
	if(!QCardLock::instance().exclusiveTryLock())
		throwException( tr("Signing/decrypting is already in progress another window."), Exception::General, __LINE__ );

	if( !d->sign.cards().contains( d->sign.card() ) || d->sign.cert().isNull() )
	{
		QCardLock::instance().exclusiveUnlock();
		throwException( tr("Signing certificate is not selected."), Exception::General, __LINE__ );
	}

	int type = NID_sha256;
	if( method == "http://www.w3.org/2001/04/xmldsig-more#rsa-sha224" ) type = NID_sha224;
	if( method == "http://www.w3.org/2001/04/xmldsig-more#rsa-sha256" ) type = NID_sha256;
	if( method == "http://www.w3.org/2001/04/xmldsig-more#rsa-sha384" ) type = NID_sha384;
	if( method == "http://www.w3.org/2001/04/xmldsig-more#rsa-sha512" ) type = NID_sha512;

	QByteArray sig;
	if( d->pkcs11 )
	{
		QPKCS11::PinStatus status = d->pkcs11->login( d->sign );
		switch( status )
		{
		case QPKCS11::PinOK: break;
		case QPKCS11::PinCanceled:
			QCardLock::instance().exclusiveUnlock();
			throwException( tr("Failed to login token") + " " + QPKCS11::errorString( status ), Exception::PINCanceled, __LINE__ );
		case QPKCS11::PinIncorrect:
			QCardLock::instance().exclusiveUnlock();
			throwException( tr("Failed to login token") + " " + QPKCS11::errorString( status ), Exception::PINIncorrect, __LINE__ );
		case QPKCS11::PinLocked:
			QCardLock::instance().exclusiveUnlock();
			reloadsign();
			throwException( tr("Failed to login token") + " " + QPKCS11::errorString( status ), Exception::PINLocked, __LINE__ );
		default:
			QCardLock::instance().exclusiveUnlock();
			throwException( tr("Failed to login token") + " " + QPKCS11::errorString( status ), Exception::General, __LINE__ );
		}

		sig = d->pkcs11->sign(type, QByteArray::fromRawData((const char*)digest.data(), int(digest.size())));
		d->pkcs11->logout();
	}
#ifdef Q_OS_WIN
	else if(d->win)
	{
		d->win->selectCert(d->sign.cert());
		sig = d->win->sign(type, QByteArray::fromRawData((const char*)digest.data(), int(digest.size())));
		if(d->win->lastError() == QWin::PinCanceled)
		{
			QCardLock::instance().exclusiveUnlock();
			throwException(tr("Failed to login token"), Exception::PINCanceled, __LINE__);
		}
	}
#endif

	QCardLock::instance().exclusiveUnlock();
	reloadsign();
	if( sig.isEmpty() )
		throwException( tr("Failed to sign document"), Exception::General, __LINE__ );
	return std::vector<unsigned char>( sig.constBegin(), sig.constEnd() );
}

void QSigner::throwException( const QString &msg, Exception::ExceptionCode code, int line ) const
{
	QString t = msg;
	Exception e( __FILE__, line, t.toUtf8().constData() );
	e.setCode( code );
	throw e;
}

TokenData QSigner::tokenauth() const { return d->auth; }
TokenData QSigner::tokensign() const { return d->sign; }

#ifdef Q_OS_WIN
void QSigner::update()
{
	if(!d->win)
		return;

	QWin::Certs certs = d->win->certs();
	for (QWin::Certs::const_iterator i = certs.constBegin(); i != certs.constEnd(); ++i)
	{
		if (d->auth.cert().isNull() && i.value() == d->auth.card() &&
			(i.key().keyUsage().contains(SslCertificate::KeyEncipherment) ||
			i.key().keyUsage().contains(SslCertificate::KeyAgreement)))
		{
			d->auth.setCert(i.key());
			Q_EMIT authDataChanged(d->auth);
		}
		if (d->sign.cert().isNull() && i.value() == d->sign.card() &&
						i.key().keyUsage().contains(SslCertificate::NonRepudiation))
		{
			d->sign.setCert(i.key());
			Q_EMIT signDataChanged(d->sign);
		}
		if(!d->auth.cert().isNull() && !d->sign.cert().isNull())
		{
			break;
		}
	}
}
#else
void QSigner::update(const SslCertificate &authCert, const SslCertificate &signCert)
{
	if(d->auth.cert().isNull())
	{
		d->auth.setCert(authCert);
		Q_EMIT authDataChanged(d->auth);
	}

	if(d->sign.cert().isNull())
	{
		d->sign.setCert(signCert);
		Q_EMIT signDataChanged(d->sign);
	}
}
#endif
