#include "sugarclient.h"

#include "dbuswinidprovider.h"
#include "enums.h"
#include "resourceconfigdialog.h"

#include <akonadi/agentfilterproxymodel.h>
#include <akonadi/agentinstance.h>
#include <akonadi/agentinstancemodel.h>
#include <akonadi/agentmanager.h>
#include <akonadi/control.h>

#include <QCloseEvent>
#include <QComboBox>
#include <QDockWidget>
#include <QInputDialog>
#include <QProgressBar>
#include <QToolBar>
#include <QTimer>

using namespace Akonadi;

SugarClient::SugarClient()
    : QMainWindow(),
      mProgressBar(0),
      mProgressBarHideTimer(0)
{
    mUi.setupUi(this);
    initialize();

    /*
     * this creates an overlay in case Akonadi is not running,
     * allowing the user to restart it
     */
    Akonadi::Control::widgetNeedsAkonadi(this);
    QMetaObject::invokeMethod(this, "slotDelayedInit", Qt::AutoConnection);

    (void)new DBusWinIdProvider(this);
}

SugarClient::~SugarClient()
{
}

void SugarClient::slotDelayedInit()
{
    Q_FOREACH (const Page *page, mPages) {
        connect(this, SIGNAL(resourceSelected(QByteArray)),
                page, SLOT(slotResourceSelectionChanged(QByteArray)));
    }

    // initialize additional UI
    mResourceSelector = createResourcesCombo();

    connect(mResourceSelector, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotResourceSelectionChanged(int)));

    mResourceDialog = new ResourceConfigDialog(this);
    connect(mResourceDialog, SIGNAL(resourceSelected(Akonadi::AgentInstance)),
            this, SLOT(slotResourceSelected(Akonadi::AgentInstance)));

    initialResourceSelection();

    connect(AgentManager::self(), SIGNAL(instanceError(Akonadi::AgentInstance,QString)),
            this, SLOT(slotResourceError(Akonadi::AgentInstance,QString)));
    connect(AgentManager::self(), SIGNAL(instanceOnline(Akonadi::AgentInstance,bool)),
            this, SLOT(slotResourceOnline(Akonadi::AgentInstance,bool)));
    connect(AgentManager::self(), SIGNAL(instanceProgressChanged(Akonadi::AgentInstance)),
            this, SLOT(slotResourceProgress(Akonadi::AgentInstance)));
}

void SugarClient::initialize()
{
    resize(900, 900);
    createMenus();
    createToolBar();
    createTabs();
    setupActions();
    mResourceSelector = 0;
    // initialize view actions
    mUi.actionSynchronize->setEnabled(false);
    mUi.actionOfflineMode->setEnabled(false);

    mProgressBar = new QProgressBar(this);
    mProgressBar->setRange(0, 100);
    mProgressBar->setMaximumWidth(100);
    statusBar()->addPermanentWidget(mProgressBar);
    mProgressBar->hide();

    mProgressBarHideTimer = new QTimer(this);
    mProgressBarHideTimer->setInterval(1000);
    connect(mProgressBarHideTimer, SIGNAL(timeout()), mProgressBar, SLOT(hide()));
}

void SugarClient::createMenus()
{
    mViewMenu = menuBar()->addMenu(tr("&View"));
}

void SugarClient::createToolBar()
{
    QToolBar *detailsToolBar = addToolBar(tr("Details Toolbar"));
    mShowDetails = new QCheckBox(tr("Show Details"));
    detailsToolBar->addWidget(mShowDetails);
    connect(mShowDetails, SIGNAL(toggled(bool)), SLOT(slotShowDetails(bool)));
}

void SugarClient::slotResourceSelectionChanged(int index)
{
    AgentInstance agent = mResourceSelector->itemData(index, AgentInstanceModel::InstanceRole).value<AgentInstance>();
    if (agent.isValid()) {
        const QString context = mResourceSelector->itemText(index);
        emit resourceSelected(agent.identifier().toLatin1());
        const QString contextTitle =
            agent.isOnline() ? QString("SugarCRM Client: %1").arg(context)
            : QString("SugarCRM Client: %1 (offline)").arg(context);
        setWindowTitle(contextTitle);
        mUi.actionSynchronize->setEnabled(true);
        mUi.actionOfflineMode->setEnabled(true);
        mUi.actionOfflineMode->setChecked(!agent.isOnline());
        mResourceDialog->resourceSelectionChanged(agent);
    } else {
        mUi.actionSynchronize->setEnabled(false);
        mUi.actionSynchronize->setEnabled(false);
    }
}

void SugarClient::slotResourceSelected(const Akonadi::AgentInstance &resource)
{
    for (int index = 0; index < mResourceSelector->count(); ++index) {
        const AgentInstance agent = mResourceSelector->itemData(index, AgentInstanceModel::InstanceRole).value<AgentInstance>();
        if (agent.isValid() && agent == resource) {
            mResourceSelector->setCurrentIndex(index);
            return;
        }
    }
}

void SugarClient::slotToggleOffline(bool offline)
{
    AgentInstance currentAgent = currentResource();
    if (currentAgent.isValid()) {
        currentAgent.setIsOnline(!offline);
    }
}

void SugarClient::slotSynchronize()
{
    AgentInstance currentAgent = currentResource();
    if (currentAgent.isValid()) {
        slotShowMessage(tr("Synchronizing with server"));
        currentAgent.synchronize();
    }
}

void SugarClient::setupActions()
{
    const QIcon reloadIcon =
        (style() != 0 ? style()->standardIcon(QStyle::SP_BrowserReload, 0, 0)
         : QIcon());
    if (!reloadIcon.isNull()) {
        mUi.actionSynchronize->setIcon(reloadIcon);
    }

    connect(mUi.actionCRMAccounts, SIGNAL(triggered()), SLOT(slotConfigureResources()));
    connect(mUi.actionOfflineMode, SIGNAL(toggled(bool)), this, SLOT(slotToggleOffline(bool)));
    connect(mUi.actionSynchronize, SIGNAL(triggered()), this, SLOT(slotSynchronize()));
    connect(mUi.actionQuit, SIGNAL(triggered()), this, SLOT(close()));

    Q_FOREACH (const Page *page, mPages) {
        connect(page, SIGNAL(statusMessage(QString)), this, SLOT(slotShowMessage(QString)));
        connect(this, SIGNAL(displayDetails()), page, SLOT(slotSetItem()));
        connect(page, SIGNAL(showDetailsChanged(bool)), this, SLOT(slotPageShowDetailsChanged()));
    }
}

void SugarClient::slotShowMessage(const QString &message)
{
    statusBar()->showMessage(message, 10000);
}

void SugarClient::createTabs()
{
    Page *page = new AccountsPage(this);
    mPages << page;
    mUi.tabWidget->addTab(page, tr("&Accounts"));
    mAccountDetailsWidget = new DetailsWidget(Account);
    page->setDetailsWidget(mAccountDetailsWidget);
    mViewMenu->addAction(page->showDetailsAction(tr("&Account Details")));

    page = new OpportunitiesPage(this);
    mPages << page;
    mUi.tabWidget->addTab(page, tr("&Opportunities"));
    mOpportunityDetailsWidget = new DetailsWidget(Opportunity);
    page->setDetailsWidget(mOpportunityDetailsWidget);
    mViewMenu->addAction(page->showDetailsAction(tr("&Opportunity Details")));

    page = new LeadsPage(this);
    mPages << page;
    mUi.tabWidget->addTab(page, tr("&Leads"));
    mLeadDetailsWidget = new DetailsWidget(Lead);
    page->setDetailsWidget(mLeadDetailsWidget);
    mViewMenu->addAction(page->showDetailsAction(tr("&Lead Details")));

    page = new ContactsPage(this);
    mPages << page;
    mUi.tabWidget->addTab(page, tr("&Contacts"));
    mContactDetailsWidget = new DetailsWidget(Contact);
    page->setDetailsWidget(mContactDetailsWidget);
    mViewMenu->addAction(page->showDetailsAction(tr("&Contact Details")));

    page = new CampaignsPage(this);
    mPages << page;
    mUi.tabWidget->addTab(page, tr("&Campaigns"));
    mCampaignDetailsWidget = new DetailsWidget(Campaign);
    page->setDetailsWidget(mCampaignDetailsWidget);
    mViewMenu->addAction(page->showDetailsAction(tr("C&ampaign Details")));

    connect(mUi.tabWidget, SIGNAL(currentChanged(int)), SLOT(slotCurrentTabChanged(int)));

    //set Accounts page as current
    mShowDetails->setChecked(mPages[ 0 ]->showsDetails());
    mUi.tabWidget->setCurrentIndex(0);
}

void SugarClient::slotConfigureResources()
{
    mResourceDialog->show();
    mResourceDialog->raise();
}

QComboBox *SugarClient::createResourcesCombo()
{
    // monitor Akonadi agents so we can check for KDCRM specific resources
    AgentInstanceModel *agentModel = new AgentInstanceModel(this);
    AgentFilterProxyModel *agentFilterModel = new AgentFilterProxyModel(this);
    agentFilterModel->setSourceModel(agentModel);
    //initialize member
    QComboBox *container = new QComboBox();
    agentFilterModel->addCapabilityFilter(QString("KDCRM").toLatin1());
    container->setModel(agentFilterModel);

    QToolBar *resourceToolBar = addToolBar(tr("CRM Account Selection"));
    resourceToolBar->addWidget(container);
    resourceToolBar->addAction(mUi.actionSynchronize);

    return container;
}

DetailsWidget *SugarClient::detailsWidget(DetailsType type)
{
    switch (type) {
    case Account:
        return mAccountDetailsWidget;
    case Opportunity:
        return mOpportunityDetailsWidget;
    case Lead:
        return mLeadDetailsWidget;
    case Contact:
        return mContactDetailsWidget;
    case Campaign:
        return mCampaignDetailsWidget;
    default:
        return 0;
    }
}

void SugarClient::closeEvent(QCloseEvent *event)
{
    QMainWindow::closeEvent(event);
}

void SugarClient::slotResourceError(const AgentInstance &resource, const QString &message)
{
    const AgentInstance currentAgent = currentResource();
    if (currentAgent.isValid() && currentAgent.identifier() == resource.identifier()) {
        slotShowMessage(message);
    }
}

void SugarClient::slotResourceOnline(const AgentInstance &resource, bool online)
{
    const AgentInstance currentAgent = currentResource();
    if (currentAgent.isValid() && currentAgent.identifier() == resource.identifier()) {
        const int index = mResourceSelector->currentIndex();
        const QString context = mResourceSelector->itemText(index);
        const QString contextTitle =
            online ? QString("SugarCRM Client: %1").arg(context)
            : QString("SugarCRM Client: %1 (offline)").arg(context);
        setWindowTitle(contextTitle);
        mUi.actionOfflineMode->setChecked(!online);
    }
}

void SugarClient::slotResourceProgress(const AgentInstance &resource)
{
    const AgentInstance currentAgent = currentResource();
    if (currentAgent.isValid() && currentAgent.identifier() == resource.identifier()) {
        const int progress = resource.progress();
        const QString message = resource.statusMessage();

        mProgressBar->show();
        mProgressBar->setValue(progress);
        if (progress == 100) {
            mProgressBarHideTimer->start();
        } else {
            mProgressBarHideTimer->stop();
        }
        statusBar()->showMessage(message, mProgressBarHideTimer->interval());
    }
}

void SugarClient::slotShowDetails(bool on)
{
    mPages[ mUi.tabWidget->currentIndex() ]->showDetails(on);
}

void SugarClient::slotPageShowDetailsChanged()
{
    mShowDetails->setChecked(mPages[ mUi.tabWidget->currentIndex() ]->showsDetails());
}

void SugarClient::slotCurrentTabChanged(int index)
{
    mShowDetails->setChecked(mPages[ index ]->showsDetails());
}

AgentInstance SugarClient::currentResource() const
{
    const int index = mResourceSelector->currentIndex();
    return mResourceSelector->itemData(index, AgentInstanceModel::InstanceRole).value<AgentInstance>();
}

void SugarClient::initialResourceSelection()
{
    const int selectors = mResourceSelector->count();
    if (selectors == 1) {
        slotResourceSelectionChanged(mResourceSelector->currentIndex());
    } else {
        mResourceSelector->setCurrentIndex(-1);
        mResourceDialog->show();
        mResourceDialog->raise();
    }
}

#include "sugarclient.moc"