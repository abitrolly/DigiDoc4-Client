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

#include "Styles.h"

#include <QFontDatabase>

#ifdef Q_OS_MAC
   #define FONT_SIZE_DECREASE 0
#else
   #define FONT_SIZE_DECREASE 4 // https://forum.qt.io/topic/26663/different-os-s-different-font-sizes/3
#endif

class FontDatabase
{
public:
	FontDatabase()
	{
		condensed = QFontDatabase::applicationFontFamilies(
			QFontDatabase::addApplicationFont(":/fonts/RobotoCondensed-Regular.ttf")
		).at(0);
		condensedBold = QFontDatabase::applicationFontFamilies(
			QFontDatabase::addApplicationFont(":/fonts/RobotoCondensed-Bold.ttf")
		).at(0);
		openSans = QFontDatabase::applicationFontFamilies(
			QFontDatabase::addApplicationFont(":/fonts/OpenSans-Regular.ttf")
		).at(0);
		regular = QFontDatabase::applicationFontFamilies(
			QFontDatabase::addApplicationFont(":/fonts/Roboto-Regular.ttf")
		).at(0);
		semiBold = QFontDatabase::applicationFontFamilies(
			QFontDatabase::addApplicationFont(":/fonts/OpenSans-SemiBold.ttf")
		).at(0);
	};
	QString fontName( Styles::Font font )
	{
		switch( font )
		{
			case Styles::OpenSansRegular: return openSans;
			case Styles::OpenSansSemiBold: return semiBold;
			case Styles::Condensed: return condensed;
			case Styles::CondensedBold: return condensedBold;
			default: return regular;
		}
	}
	QFont font(Styles::Font font, int size)
	{
		return QFont( fontName( font ), size - FONT_SIZE_DECREASE );
	};

private:
	QString condensed;
	QString condensedBold;
	QString openSans;
	QString regular;
	QString semiBold;
};

QFont Styles::font(Styles::Font font, int size)
{
	static FontDatabase fontDatabase;
	return fontDatabase.font(font, size);
}
