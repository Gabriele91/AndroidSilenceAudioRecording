#include "q_android_silence_audio_recording.h"
#include "ui_q_android_silence_audio_recording.h"
#include <QWindow>
#include <QMouseEvent>
#include <QMessageBox>

q_android_silence_audio_recording::q_android_silence_audio_recording(QWidget *parent)
:QMainWindow(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint)
,m_ui(new Ui::q_android_silence_audio_recording)
,m_options(new q_options(this))
,m_settings(new q_settings(this))
{
    //init ui
    m_ui->setupUi(this);

    ////////////////////////////////////////////////////////////////////////////
    //set settings
    m_ui->m_w_info->layout()->addWidget(m_settings);
    m_settings->hide();
    ////////////////////////////////////////////////////////////////////////////
    //set style
    m_ui->m_hl_top_menu->setStyleSheet(tr("QFrame[accessibleName=\"top_frame\"] "
                                          "{"
                                          "border: 1px solid #3A3939;"
                                          "color: silver;"
                                          "margin-bottom: 6px;"
                                          "padding: 0px"
                                          "}"
                                          ));
    m_ui->m_w_info->setStyleSheet(tr("QWidget[accessibleName=\"bottom_frame\"] "
                                     "{"
                                     "border: 1px solid #3A3939;"
                                     "color: silver;"
                                     "}"
                                     ));
    //show info
    setContextMenuPolicy(Qt::ActionsContextMenu);
    setToolTip(tr("Drag the application with the left mouse button.\n"
                  "Use the right mouse button to open a context menu."));
    setWindowTitle(tr("Android Silence Audio Recording"));
    ////////////////////////////////////////////////////////////////////////////
    //events
    //exit menu
    QAction *l_quit_action = new QAction(tr("E&xit"), this);
    l_quit_action->setShortcut(tr("Ctrl+Q"));
    connect(l_quit_action, SIGNAL(triggered()), qApp, SLOT(quit()));
    addAction(l_quit_action);
    //connect exit
    connect(m_ui->m_cb_exit, SIGNAL(stateChanged(int)), qApp, SLOT(quit()));
    //connect minimized
    connect(m_ui->m_cb_minimized, &QCheckBox::stateChanged, this,
    [this](int changed)
    {
        if(!changed)  showMinimized();
        else          showNormal();
    });
    //massimize
    m_ui->m_hl_top_menu->set_callback(
    [this](bool first)
    {
        if(first)  showMaximized();
        else       showNormal();
    });
    //connect select item
    connect(m_ui->m_lw_devices,
            SIGNAL(itemClicked(QListWidgetItem*)),
            this,
            SLOT(itemClicked(QListWidgetItem*)));
    ////////////////////////////////////////////////////////////////////////////
    //set view list
    m_list_listener.set_list(m_ui->m_lw_devices);
    //init rak server
    m_rak_server.init(m_options->get_port(),
                      m_options->get_max_clients());
    //set callback
    m_rak_server.loop(m_list_listener);
}

q_android_silence_audio_recording::~q_android_silence_audio_recording()
{
    //stop server
    m_rak_server.stop_loop();
    //dealloc dialogs
    delete m_options;
    //delete ui
    delete m_ui;
}

//get rak server
rak_server& q_android_silence_audio_recording::get_rak_server()
{
    return m_rak_server;
}
//change ui
void q_android_silence_audio_recording::show_settings(q_audio_server_listener* listener)
{
    //disable signals
    m_settings->blockSignals(true);
    m_ui->m_lw_devices->blockSignals(true);
    //set listener
    m_settings->set_audio_server_listener(listener,m_options->get_path());
    //change ui
    m_ui->m_lw_devices->hide();
    m_ui->m_pb_options->hide();
    m_settings->show();
    //renable signals
    m_settings->blockSignals(false);
    m_ui->m_lw_devices->blockSignals(false);
}

void q_android_silence_audio_recording::show_list_device()
{
    //disable signals
    m_settings->blockSignals(true);
    m_ui->m_lw_devices->blockSignals(true);
    //change ui
    m_settings->hide();
    //show
    m_ui->m_lw_devices->show();
    m_ui->m_pb_options->show();
    //renable signals
    m_settings->blockSignals(false);
    m_ui->m_lw_devices->blockSignals(false);
    //rebuild list
    m_list_listener.update_all_item_status();
    m_ui->m_lw_devices->repaint();
}

void q_android_silence_audio_recording::itemClicked(QListWidgetItem* item)
{
    //get value
    QVariant variant=item->data(Qt::UserRole);
    //get row
    auto* row = (q_list_server_listener::row_map_listener*)variant.value<void*>();
    //show
    show_settings(&row->m_listener);
}

void q_android_silence_audio_recording::options()
{
    //exec
    int ret_value = m_options->exec();
    //test port
    if(ret_value = QMessageBox::Ok &&
       m_options->get_port() != m_rak_server.get_init_port() )
    {
        //destoy all
        m_rak_server.shutdown();
        //clear list
        m_list_listener.clear();
        //init rak server
        m_rak_server.init(m_options->get_port(),
                          m_options->get_max_clients());
        //set callback
        m_rak_server.loop(m_list_listener);
    }
}

void q_android_silence_audio_recording::showEvent(QShowEvent * event)
{
    m_ui->m_cb_minimized->setChecked(true);
}

void q_android_silence_audio_recording::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        move(event->globalPos() - m_drag_position);
        event->accept();
    }
}
void q_android_silence_audio_recording::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_drag_position = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}
