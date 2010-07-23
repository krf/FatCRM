#include "accountdetails.h"

#include <akonadi/item.h>

#include <kdcrmdata/sugaraccount.h>

using namespace Akonadi;


AccountDetails::AccountDetails( QWidget *parent )
    : QWidget( parent )

{
    mUi.setupUi( this );
    initialize();
}

AccountDetails::~AccountDetails()
{
}

void AccountDetails::initialize()
{
    QList<QLineEdit*> lineEdits =  mUi.accountInformationGB->findChildren<QLineEdit*>();
    Q_FOREACH( QLineEdit* le, lineEdits )
        connect( le, SIGNAL( textChanged( const QString& ) ),
                 this, SLOT( slotEnableSaving() ) );

    QList<QComboBox*> comboBoxes =  mUi.accountInformationGB->findChildren<QComboBox*>();
    Q_FOREACH( QComboBox* cb, comboBoxes )
        connect( cb, SIGNAL( currentIndexChanged( int ) ),
                 this, SLOT( slotEnableSaving() ) );

    connect( mUi.description, SIGNAL( textChanged() ),
             this,  SLOT( slotEnableSaving() ) );

    connect( mUi.saveButton, SIGNAL( clicked() ),
             this, SLOT( slotSaveAccount() ) );
    mUi.saveButton->setEnabled( false );
}

void AccountDetails::reset()
{
    QList<QLineEdit*> lineEdits =  mUi.accountInformationGB->findChildren<QLineEdit*>();
    Q_FOREACH( QLineEdit* le, lineEdits )
        connect( le, SIGNAL( textChanged( const QString& ) ),
                 this, SLOT( slotEnableSaving() ) );

    QList<QComboBox*> comboBoxes =  mUi.accountInformationGB->findChildren<QComboBox*>();
    Q_FOREACH( QComboBox* cb, comboBoxes )
        disconnect( cb, SIGNAL( currentIndexChanged( int ) ),
                    this, SLOT( slotEnableSaving() ) );

    disconnect( mUi.description, SIGNAL( textChanged() ),
                this,  SLOT( slotEnableSaving() ) );
}

void AccountDetails::setItem (const Item &item )
{
    // new item selected reset flag and saving
    mModifyFlag = true;
    reset();

    // account info
    const SugarAccount account = item.payload<SugarAccount>();
    mUi.name->setText( account.name() );
    mUi.website->setText( account.website() );
    mUi.tyckerSymbol->setText( account.tyckerSymbol() );
    mUi.parentName->setCurrentIndex( mUi.parentName->findText( account.parentName() ) );
    mUi.ownership->setText( account.ownership() );
    mUi.industry->setCurrentIndex(mUi.industry->findText( account.industry() ) );
    mUi.accountType->setCurrentIndex(mUi.accountType->findText( account.accountType() ) );
    mUi.campaignName->setCurrentIndex(mUi.campaignName->findText( account.campaignName() ) );
    mUi.assignedUserName->setCurrentIndex(mUi.assignedUserName->findText( account.assignedUserName() ) );
    mUi.phoneOffice->setText( account.phoneOffice() );
    mUi.phoneFax->setText( account.phoneFax() );
    mUi.phoneAlternate->setText( account.phoneAlternate() );
    mUi.employees->setText( account.employees() );
    mUi.rating->setText( account.rating() );
    mUi.sicCode->setText( account.sicCode() );
    mUi.annualRevenue->setText( account.annualRevenue() );
    mUi.email1->setText( account.email1() );
    mUi.billingAddressStreet->setText( account.billingAddressStreet() );
    mUi.billingAddressCity->setText( account.billingAddressCity() );
    mUi.billingAddressState->setText( account.billingAddressState() );
    mUi.billingAddressPostalcode->setText( account.billingAddressPostalcode() );
    mUi.billingAddressCountry->setText( account.billingAddressCountry() );
    mUi.shippingAddressStreet->setText( account.shippingAddressStreet() );
    mUi.shippingAddressCity->setText( account.shippingAddressCity() );
    mUi.shippingAddressState->setText( account.shippingAddressState() );
    mUi.shippingAddressPostalcode->setText( account.shippingAddressPostalcode() );
    mUi.shippingAddressCountry->setText( account.shippingAddressCountry() );
    mUi.description->setPlainText( account.description() );
    mUi.modifiedBy->setText( account.modifiedByName() );
    mUi.modifiedBy->setProperty( "modifiedUserId", qVariantFromValue<QString>( account.modifiedUserId() ) );
    mUi.modifiedDate->setText( account.dateModified() );
    mUi.createdDate->setText( account.dateEntered() );
    mUi.createdDate->setProperty( "id", qVariantFromValue<QString>( account.id( ) ) );
    mUi.createdDate->setProperty( "deleted",  qVariantFromValue<QString>( account.deleted( ) ) );
    mUi.createdBy->setText( account.createdByName() );
    mUi.createdBy->setProperty( "createdBy", qVariantFromValue<QString>( account.createdBy( ) ) );
    initialize();
}

void AccountDetails::clearFields ()
{
    // reset line edits
    QList<QLineEdit*> lineEdits =
        mUi.accountInformationGB->findChildren<QLineEdit*>();
    Q_FOREACH( QLineEdit* le, lineEdits )
        le->setText(QString());

    // reset label and properties
    QList<QLabel*> labels =
        mUi.accountInformationGB->findChildren<QLabel*>();
    Q_FOREACH( QLabel* lab, labels ) {
        QString value = lab->objectName();
        if ( value == "modifiedDate" )
            lab->clear();
        else if ( value == "modifiedBy" ) {
            lab->clear();
            lab->setProperty( "modifiedUserId", qVariantFromValue<QString>( QString() ) );
        }
        else if ( value == "createdDate" ) {
            lab->clear();
            lab->setProperty( "id", qVariantFromValue<QString>( QString() ) );
            lab->setProperty( "deleted", qVariantFromValue<QString>( QString() ) );
        }
        else if ( value == "createdBy" ) {
            lab->clear();
            lab->setProperty( "createdBy", qVariantFromValue<QString>( QString() ) );
        }
    }

    // reset combos
    QList<QComboBox*> comboBoxes =
        mUi.accountInformationGB->findChildren<QComboBox*>();
    Q_FOREACH( QComboBox* cb, comboBoxes )
        cb->setCurrentIndex( 0 );

    // initialize other fields
    mUi.description->clear();
    mUi.name->setFocus();


    // we are creating a new account
    slotSetModifyFlag( false );
}


void AccountDetails::slotSetModifyFlag( bool value )
{
    mModifyFlag = value;
}

void AccountDetails::slotEnableSaving()
{
    // check for modify flag
    mUi.saveButton->setEnabled( true );
}

void AccountDetails::slotSaveAccount()
{
    if ( !mData.empty() )
        mData.clear();

    mUi.modifiedDate->setText( QDateTime::currentDateTime().toString( QString( "yyyy-MM-dd hh:mm:ss") ) );

    QList<QLineEdit*> lineEdits =
        mUi.accountInformationGB->findChildren<QLineEdit*>();
    Q_FOREACH( QLineEdit* le, lineEdits )
        mData[le->objectName()] = le->text();

    QList<QLabel*> labels =
        mUi.accountInformationGB->findChildren<QLabel*>();
    Q_FOREACH( QLabel* lab, labels ) {
        QString objName = lab->objectName();
        if ( objName == "modifiedDate" )
            mData["modifiedDate"] = lab->text();
        else if ( objName == "modifiedBy" ) {
            mData["modifiedBy"] = lab->text();
            mData["modifiedUserId"] = lab->property( "modifiedUserId" ).toString();
            mData["modifiedUserName"] = lab->property( "modifiedUserName" ).toString();
        }
        else if ( objName == "createdDate" ) {
            mData["createdDate"] = lab->text();
            mData["id"] = lab->property( "id" ).toString();
            mData["deleted"] = lab->property( "deleted" ).toString();
        }
        else if ( objName == "createdBy" ) {
            mData["createdByName"] = lab->text();
            mData["createdBy"] = lab->property( "createdBy" ).toString();
        }
    }
    mData["industry"] = mUi.industry->currentText();
    mData["accountType"] = mUi.accountType->currentText();
    mData["parentName"] = mUi.parentName->currentText();
    mData["parentId"] = mAccountsData.value( mUi.parentName->currentText() );
    mData["campaignName"] = mUi.campaignName->currentText();
    mData["campaignId"] = mCampaignsData.value( mUi.campaignName->currentText() );
    mData["assignedUserName"] = mUi.assignedUserName->currentText();
    mData["assignedUserId"] = mAssignedToData.value( mUi.assignedUserName->currentText() );
    mData["description"] = mUi.description->toPlainText();

    if ( !mModifyFlag )
        emit saveAccount();
    else
        emit modifyAccount();
}

void AccountDetails::addAccountData( const QString &accountName,  const QString &accountId )
{
    mAccountsData.insert( accountName, accountId );
    if ( mUi.parentName->findText( accountName ) < 0 )
        mUi.parentName->addItem( accountName );
}

void AccountDetails::removeAccountData( const QString &accountName )
{
    mAccountsData.remove( accountName );
    int index = mUi.parentName->findText( accountName );
    if ( index > 0 )
        mUi.parentName->removeItem( index );
}

void AccountDetails::addCampaignData( const QString &campaignName,  const QString &campaignId )
{
    mCampaignsData.insert( campaignName, campaignId );
    if ( mUi.campaignName->findText( campaignName ) < 0 )
        mUi.campaignName->addItem( campaignName );
}

void AccountDetails::removeCampaignData( const QString &campaignName )
{
    mCampaignsData.remove( campaignName );
    int index = mUi.campaignName->findText( campaignName );
    if ( index > 0 )
        mUi.campaignName->removeItem( index );
}

void AccountDetails::addAssignedToData( const QString &name, const QString &id )
{
    mAssignedToData.insert( name, id );
    if ( mUi.assignedUserName->findText( name ) < 0 )
        mUi.assignedUserName->addItem( name );
}


