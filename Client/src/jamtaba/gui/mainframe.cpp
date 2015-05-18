#include "MainFrame.h"
#include <QCloseEvent>
#include "PreferencesDialog.h"
#include <QDebug>
#include <QDesktopWidget>
#include <QLayout>
#include <QList>
#include <QAction>
#include "JamRoomViewPanel.h"
#include "../persistence/ConfigStore.h"
#include "../JamtabaFactory.h"
#include "../audio/core/AudioDriver.h"
#include "../midi/MidiDriver.h"
#include "../MainController.h"
#include "../loginserver/LoginService.h"
#include "../loginserver/JamRoom.h"
#include "../audio/core/plugins.h"
#include "LocalTrackView.h"
#include "plugins/guis.h"
#include "FxPanel.h"
#include "../audio/vst/PluginFinder.h"
#include "pluginscandialog.h"

using namespace Audio;
using namespace Persistence;
using namespace Controller;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MainFrame::MainFrame(Controller::MainController *mainController, QWidget *parent)
    : QMainWindow(parent),
    fxMenu(nullptr), mainController(mainController), pluginScanDialog(nullptr)
{
	ui.setupUi(this);

    if(ConfigStore::windowWasMaximized()){
        setWindowState(Qt::WindowMaximized);
    }
    else{
        QPointF location = ConfigStore::getLastWindowLocation();
        QDesktopWidget* desktop = QApplication::desktop();
        int desktopWidth = desktop->width();
        int desktopHeight = desktop->height();
        int x = desktopWidth * location.x();
        int y = desktopHeight * location.y();
        this->move(x, y);
    }
    timerID = startTimer(50);


    Login::LoginService* loginService = this->mainController->getLoginService();
    connect(loginService, SIGNAL(connectedInServer(QList<Login::AbstractJamRoom*>)),
                                 this, SLOT(on_connectedInServer(QList<Login::AbstractJamRoom*>)));

    fxMenu = createFxMenu();

    localTrackView = new LocalTrackView(this, this->mainController);
    ui.verticalLayoutLeft->addWidget(localTrackView);
    localTrackView->initializeFxPanel(fxMenu);

    QObject::connect(localTrackView, SIGNAL(editingPlugin(Audio::Plugin*)), this, SLOT(on_editingPlugin(Audio::Plugin*)));
    QObject::connect(localTrackView, SIGNAL(removingPlugin(Audio::Plugin*)), this, SLOT(on_removingPlugin(Audio::Plugin*)));

    const Vst::PluginFinder* pluginFinder = mainController->getPluginFinder();
    QObject::connect(pluginFinder, SIGNAL(scanStarted()), this, SLOT(onPluginScanStarted()));
    QObject::connect(pluginFinder, SIGNAL(scanFinished()), this, SLOT(onPluginScanFinished()));
    QObject::connect(pluginFinder, SIGNAL(vstPluginFounded(Audio::PluginDescriptor*)), this, SLOT(onPluginFounded(Audio::PluginDescriptor*)));

    //QObject::connect( ui.menuTest, SIGNAL(triggered(QAction*)), this, SLOT(scanTest(QAction*)));

    QStringList vstPaths = ConfigStore::getVstPluginsPaths();
    if(vstPaths.empty()){//no vsts in database cache, try scan
        mainController->scanPlugins();
    }
    else{//use vsts stored in settings file
        mainController->initializePluginsList(vstPaths);
        onPluginScanFinished();
    }

    QObject::connect(ui.menuAudioPreferences, SIGNAL(triggered()), this, SLOT(on_preferencesClicked()));
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void MainFrame::on_removingPlugin(Audio::Plugin *plugin){
    PluginWindow* window = PluginWindow::getWindow(this, plugin);
    if(window){
        window->close();
    }
    mainController->removePlugin(plugin);

}

void MainFrame::on_editingPlugin(Audio::Plugin *plugin){
    showPluginGui(plugin);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void MainFrame::on_fxMenuActionTriggered(QAction* action){
    //add a new plugin
    Audio::PluginDescriptor* pluginDescriptor = action->data().value<Audio::PluginDescriptor*>();
    Audio::Plugin* plugin = mainController->addPlugin(pluginDescriptor);
    localTrackView->addPlugin(plugin);
    showPluginGui(plugin);
}
//++++++++++++++++++++++++++++++++++++
void MainFrame::showPluginGui(Audio::Plugin *plugin){
    PluginWindow* window = PluginWindow::getWindow(this, plugin);

    if(!window->isVisible()){
        window->show();//show to generate a window handle, VST plugins use this handle to draw plugin GUI
        int editorLeft = localTrackView->x() + localTrackView->width();
        int editorTop = height()/4;
        plugin->openEditor(window, mapToGlobal(QPoint(editorLeft, editorTop)));
    }
    else{
      window->activateWindow();
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//PluginGui* MainFrame::createPluginView(Plugin::PluginDescriptor* d, Audio::Plugin* plugin){
//    if(d->getGroup() == "Jamtaba"){
//        if(d->getName() == "Delay"){
//            return new DelayGui((Plugin::JamtabaDelay*)plugin);
//        }
//    }
//    return nullptr;
//}

//++++++++++++++++++++
QMenu* MainFrame::createFxMenu(){
    if(fxMenu ){
        fxMenu->close();
        fxMenu->deleteLater();
    }
    QMenu* menu = new QMenu(this);

    std::vector<Audio::PluginDescriptor*> plugins = mainController->getPluginsDescriptors();
    for(Audio::PluginDescriptor* pluginDescriptor  : plugins){
        QAction* action = menu->addAction(pluginDescriptor->getName());
        action->setData(QVariant::fromValue(pluginDescriptor));
    }

    menu->connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(on_fxMenuActionTriggered(QAction*)));
    return menu;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//esses eventos deveriam ser tratados no controller
void MainFrame::on_connectedInServer(QList<Login::AbstractJamRoom*> rooms){
    ui.allRoomsContent->setLayout(new QVBoxLayout(ui.allRoomsContent));
    foreach(Login::AbstractJamRoom* room, rooms){
        JamRoomViewPanel* roomViewPanel = new JamRoomViewPanel(room, ui.allRoomsContent);
        roomViewPanels.insert(room, roomViewPanel);
        ui.allRoomsContent->layout()->addWidget(roomViewPanel);
        connect( roomViewPanel, SIGNAL(startingListeningTheRoom(Login::AbstractJamRoom*)), this, SLOT(on_startingRoomStream(Login::AbstractJamRoom*)));
        connect( roomViewPanel, SIGNAL(finishingListeningTheRoom(Login::AbstractJamRoom*)), this, SLOT(on_stoppingRoomStream(Login::AbstractJamRoom*)));
    }
}

void MainFrame::on_startingRoomStream(Login::AbstractJamRoom* room){
    if(room->hasStreamLink()){
        mainController->playRoomStream(room);
    }
}

void MainFrame::on_stoppingRoomStream(Login::AbstractJamRoom * room){
    mainController->stopRoomStream();
    roomViewPanels[room]->clearPeaks();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void MainFrame::timerEvent(QTimerEvent *){

    //update local input track peaks
    Peaks inputPeaks = mainController->getInputPeaks();
    localTrackView->setPeaks(inputPeaks.left, inputPeaks.right);

    //update room stream plot
    if(mainController->isPlayingRoomStream()){
          Login::AbstractJamRoom* room = mainController->getCurrentStreamingRoom();
          JamRoomViewPanel* roomView =  roomViewPanels[room];
          Controller::Peaks peaks = mainController->getRoomStreamPeaks();
          roomView->addPeak(peaks.max());
    }
}

//++++++++++++=
void MainFrame::changeEvent(QEvent *ev){
    if(ev->type() == QEvent::WindowStateChange){
        ConfigStore::storeWindowState(isMaximized());
    }
    ev->accept();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void MainFrame::closeEvent(QCloseEvent *)
 {
    //qDebug() << "MainFrame::closeEvent";
    if(mainController != NULL){
        mainController->stop();
    }
    //store window position
    QRect screen = QApplication::desktop()->screenGeometry();
    float x = (float)this->pos().x()/screen.width();
    float y = (float)this->pos().y()/screen.height();
    QPointF location(x, y) ;
    ConfigStore::storeWindowLocation( location ) ;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void MainFrame::showEvent(QShowEvent *)
{
    mainController->start();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MainFrame::~MainFrame()
{
    killTimer(timerID);
    //delete mainController;
    //delete peakMeter;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//void MainFrame::on_freshDataReceivedFromLoginServer(const Login::LoginServiceParser &response){
//    if(ui.allRoomsContent->layout() == 0){
//        QLayout* layout = new QVBoxLayout(ui.allRoomsContent);
//        layout->setSpacing(20);
//        ui.allRoomsContent->setLayout(layout);
//    }
//    foreach (Model::AbstractJamRoom* room, response.getRooms()) {
//        QWidget* widget = new JamRoomViewPanel(room, this->ui.allRoomsContent);
//        ui.allRoomsContent->layout()->addWidget(widget);

//    }
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// preferences menu
void MainFrame::on_preferencesClicked()
{
    qDebug() << "disparou acao";;
    AudioDriver* driver = mainController->getAudioDriver();
    driver->stop();
    PreferencesDialog dialog(mainController, this);
    connect(&dialog, SIGNAL(audioIOPropertiesChanged(int, int,int,int,int,int,int,int)), this, SLOT(on_IOPropertiesChanged(int, int,int,int,int,int,int,int)));
    dialog.exec();
    driver->start();
    //audio driver is restarted in on_audioIOPropertiesChanged. This slot is always invoked when AudioIODialog is closed.
}

void MainFrame::on_IOPropertiesChanged(int /*midiDevice*/, int audioDevice, int firstIn, int lastIn, int firstOut, int lastOut, int sampleRate, int bufferSize)
{
    Audio::AudioDriver* audioDriver = mainController->getAudioDriver();
#ifdef _WIN32
    audioDriver->setProperties(audioDevice, firstIn, lastIn, firstOut, lastOut, sampleRate, bufferSize);

#else
    //preciso de um outro on_audioIoPropertiesChanged que me dê o input e o output device
    //audioDriver->setProperties(selectedDevice, firstIn, lastIn, firstOut, lastOut, sampleRate, bufferSize);
#endif
    //TODO setar o midi device selecionado
    //mainController->getMidiDriver()->
    ConfigStore::storeAudioSettings(firstIn, lastIn, firstOut, lastOut, audioDevice, audioDevice, sampleRate, bufferSize);
}

//plugin finder events
void MainFrame::onPluginScanStarted(){
    if(!pluginScanDialog){
        pluginScanDialog = new PluginScanDialog(this);
    }
    pluginScanDialog->show();
}

void MainFrame::onPluginScanFinished(){
    if(pluginScanDialog){
        pluginScanDialog->close();
    }
    delete pluginScanDialog;
    pluginScanDialog = nullptr;


    fxMenu = createFxMenu();
    localTrackView->initializeFxPanel(fxMenu);
}

void MainFrame::onPluginFounded(Audio::PluginDescriptor* descriptor){
    if(pluginScanDialog){
        pluginScanDialog->setText(descriptor->getPath());
    }
}



