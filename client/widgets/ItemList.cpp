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

#include "ItemList.h"
#include "ui_ItemList.h"
#include "Styles.h"

#include <vector>

ItemList::ItemList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ItemList)
{
    ui->setupUi(this);
    ui->add->init(LabelButton::DeepCerulean | LabelButton::WhiteBackground, "+ Lisa veel faile", "#add-file");
    ui->listHeader->setFont( Styles::instance().font(Styles::OpenSansSemiBold, 14));
    ui->add->setFont(Styles::instance().font(Styles::OpenSansRegular, 13));

    connect(ui->add, &QLabel::linkActivated, this, &ItemList::add);
}

ItemList::~ItemList()
{
    delete ui;
}

void ItemList::add(const QString &anchor)
{
    QWidget* item = new QWidget;
    item->setMinimumSize(460, 40);
    item->setStyleSheet("border: solid #c8c8c8; border-width: 1px 0px 1px 0px; background-color: #fafafa; color: #000000; text-decoration: none solid rgb(0, 0, 0);");
    ui->itemLayout->insertWidget(items.size(), item);
    item->show();
    items.push_back(item);
}