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

#pragma once

#include "common_enums.h"

#include <QWidget>

namespace Ui {
class MainAction;
}

class MainAction : public QWidget
{
	Q_OBJECT

public:
	explicit MainAction(ria::qdigidoc4::Actions action, QWidget *parent, bool showSelector = true);
	~MainAction();

	void update(ria::qdigidoc4::Actions action, bool showSelector = true);

signals:
	void action( int action );
	void dropdown();

protected:
	void changeEvent(QEvent* event) override;

private:
	void update();

	Ui::MainAction *ui;
	ria::qdigidoc4::Actions actionType;
};
