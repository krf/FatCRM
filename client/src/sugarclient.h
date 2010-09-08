#ifndef SUGARCLIENT_H
#define SUGARCLIENT_H

#include "ui_mainwindow.h"
#include <accountdetails.h>
#include <opportunitydetails.h>
#include <leaddetails.h>
#include <campaigndetails.h>
#include <contactdetails.h>
#include <accountspage.h>
#include <opportunitiespage.h>
#include <leadspage.h>
#include <contactspage.h>
#include <campaignspage.h>
#include "enums.h"

#include <QMainWindow>

class QAction;
class QToolBar;

namespace Akonadi {
    class AgentInstance;
}

class SugarClient : public QMainWindow
{
    Q_OBJECT
public:
    SugarClient();

    ~SugarClient();

    inline void displayDockWidgets( bool value = true )
    { mUi.showDetails->setChecked( value ); }
    QWidget *detailsWidget( DetailsType type );

Q_SIGNALS:
    void resourceSelected( const QByteArray &identifier );
    void displayDetails();

protected:
    void closeEvent( QCloseEvent *event );

private:
    void initialize();
    void createMenus();
    void createToolBar();
    void createDockWidgets();
    void setupActions();
    void createTabs();

    QMenu *mViewMenu;
    QToolBar *mToolBar;
    QDockWidget *mAccountDetailsDock;
    AccountDetails *mAccountDetailsWidget;
    QAction *mViewAccountAction;
    QDockWidget *mOpportunityDetailsDock;
    OpportunityDetails *mOpportunityDetailsWidget;
    QAction *mViewOpportunityAction;
    QDockWidget *mLeadDetailsDock;
    LeadDetails *mLeadDetailsWidget;
    QAction *mViewLeadAction;
    QDockWidget *mContactDetailsDock;
    ContactDetails *mContactDetailsWidget;
    QAction *mViewContactAction;
    QDockWidget *mCampaignDetailsDock;
    CampaignDetails *mCampaignDetailsWidget;
    QAction *mViewCampaignAction;
    AccountsPage *mAccountsPage;
    OpportunitiesPage *mOpportunitiesPage;
    LeadsPage *mLeadsPage;
    ContactsPage *mContactsPage;
    CampaignsPage *mCampaignsPage;

    QComboBox * mResourceSelector; // Login;

    Ui_MainWindow mUi;

private Q_SLOTS:
    void slotDelayedInit();
    void slotReload();
    void slotResourceSelectionChanged( int index );
    void slotShowMessage( const QString& );
    void slotManageItemDetailsView( int currentTab );
    void slotManageDetailsDisplay( bool value );
    void slotDetailsDisplayDisabled( bool value );
    void slotLogin();
    void slotConfigureResources();
    void slotResourceError( const Akonadi::AgentInstance &resource, const QString &message );
    void slotResourceOnline( const Akonadi::AgentInstance &resource, bool online );

private:
    void detachDockViews( bool value );
    QComboBox* getResourcesCombo();
};

#endif
