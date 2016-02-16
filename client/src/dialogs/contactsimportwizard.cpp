/*
  This file is part of FatCRM, a desktop application for SugarCRM written by KDAB.

  Copyright (C) 2015-2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Authors: Tobias Koenig <tobias.koenig@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "contactsimportwizard.h"

#include "accountimportpage.h"
#include "contactsimportpage.h"
#include "kjobprogresstracker.h"

#include <AkonadiCore/ItemCreateJob>
#include <AkonadiCore/ItemModifyJob>
#include <KLocale>

ContactsImportWizard::ContactsImportWizard(QWidget *parent)
    : QWizard(parent)
    , mAccountImportPage(new AccountImportPage)
    , mContactsImportPage(new ContactsImportPage)
{
    setAttribute(Qt::WA_DeleteOnClose);

    addPage(mAccountImportPage);
    addPage(mContactsImportPage);

    connect(mAccountImportPage, SIGNAL(chosenContactsAvailable(QVector<ContactsSet>)),
            mContactsImportPage, SLOT(setChosenContacts(QVector<ContactsSet>)));
    connect(mContactsImportPage, SIGNAL(importedItems(QVector<Akonadi::Item>)),
            this, SLOT(importItems(QVector<Akonadi::Item>)));
    connect(mContactsImportPage, SIGNAL(layoutChanged()),
            this, SLOT(adaptWindowSize()));

    setWindowTitle(i18n("Import Contacts"));
}

void ContactsImportWizard::setAccountCollection(const Akonadi::Collection &collection)
{
    mAccountImportPage->setAccountCollection(collection);
}

void ContactsImportWizard::setContactsCollection(const Akonadi::Collection &collection)
{
    mContactsCollection = collection;
}

void ContactsImportWizard::setImportedContacts(const QVector<ContactsSet> &contacts)
{
    mAccountImportPage->setImportedContacts(contacts);
}

void ContactsImportWizard::setContactsModel(ItemsTreeModel *model)
{
    mContactsImportPage->setContactsModel(model);
}

void ContactsImportWizard::accept()
{
    QWizard::accept();
}

void ContactsImportWizard::reject()
{
    mAccountImportPage->cleanup();
    mContactsImportPage->cleanup();
    QWizard::reject();
}

void ContactsImportWizard::adaptWindowSize()
{
    resize(mContactsImportPage->width() + 40, height());
}

void ContactsImportWizard::importItems(const QVector<Akonadi::Item> &items)
{
    if (items.isEmpty())
        return;

    setAttribute(Qt::WA_DeleteOnClose, false);

    KJobProgressTracker *tracker = new KJobProgressTracker(this, this);
    tracker->setCaption(i18n("Import Contacts"));
    tracker->setLabel(i18n("Importing contacts, please wait..."));
    connect(tracker, SIGNAL(finished()), SLOT(deleteLater()));

    foreach (const Akonadi::Item &item, items) {
        const KContacts::Addressee contact = item.payload<KContacts::Addressee>();
        if (item.isValid()) {
            Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(item, this);

            const QString errorMessage = i18n("Unable to update contact %1: %2", contact.realName());
            tracker->addJob(job, errorMessage);
        } else {
            Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob(item, mContactsCollection, this);

            const QString errorMessage = i18n("Unable to create contact %1: %2", contact.realName());
            tracker->addJob(job, errorMessage);
        }
    }

    tracker->start();
}