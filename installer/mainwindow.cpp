
#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "modules/dialogs/dialogmsg.h"
#include "modules/dialogs/dialogbool.h"

#include <QFontDatabase>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // load the font
    const QString font_family = QFontDatabase::applicationFontFamilies(
        QFontDatabase::addApplicationFont(":/fonts/Metropolis")).at(0);
    const QString paths_font_family = QFontDatabase::applicationFontFamilies(
        QFontDatabase::addApplicationFont(":/fonts/Hack")).at(0);
    // initialize the fonts
    this->FONT_main = QFont(
        font_family,
        13 );
    this->FONT_bigger = QFont(
        font_family,
        15 );
    this->FONT_paths = QFont(
        paths_font_family,
        13 );
    // apply the fonts
    this->ui->menubar->setFont( this->FONT_main );
    this->ui->centralwidget->setFont( this->FONT_main );
    this->ui->frame_Steps->setFont( this->FONT_bigger );
    this->ui->label_Paths_Executable->setFont( this->FONT_paths );
    this->ui->label_Paths_AppData->setFont( this->FONT_paths );
    this->ui->label_Paths_ConfigFile->setFont( this->FONT_paths );

    // color palettes
    this->PALETTE_norm = QPalette();
    this->PALETTE_step = QPalette();
    this->PALETTE_step.setColor( QPalette::ColorRole::Window, this->PALETTE_step.highlight().color() );

    // languages
    connect( this->ui->actionEnglish,  &QAction::triggered, this, &MainWindow::menu_actionEnglish_triggered  );
    connect( this->ui->actionEspanol,  &QAction::triggered, this, &MainWindow::menu_actionEspanol_triggered  );
    connect( this->ui->actionFrancais, &QAction::triggered, this, &MainWindow::menu_actionFrancais_triggered );
    connect( this->ui->actionItaliano, &QAction::triggered, this, &MainWindow::menu_actionItaliano_triggered );

    // init steps labels
    this->ui->label_Install_Step1->setAutoFillBackground( true );
    this->ui->label_Install_Step2->setAutoFillBackground( true );

    // steps
    this->step = 1;
    this->ui->stacked_Main->setCurrentIndex( 0 );
    this->ui->stacked_Pages->setCurrentIndex( 0 );

    // step 1
    this->ui->button_Back->setVisible( false );
    this->ui->label_Install_Step1->setPalette( this->PALETTE_step );
    this->ui->label_Paths_Executable->setText( QString::fromStdString( this->exec_path.string() ) );
    this->ui->label_Paths_ConfigFile->setText( QString::fromStdString( this->conf_path.string() ) );
    this->ui->label_Paths_AppData->setText(    QString::fromStdString( this->data_path.string() ) );

}

MainWindow::~MainWindow()
{
    delete this->ui;
    delete this->waiter_timer;
    delete this->installer_timer;
}


///////////////
//// UTILS ////
///////////////
const std::string MainWindow::cleanPath( const QString& path )
{
    if ( path.endsWith('/') || path.endsWith('\\') ) {
        return path.toStdString().substr( 0, path.size()-1 );
    } else {
        return path.toStdString();
    }
}



//////////////
//// MENU ////
/// //////////
// switch language
void MainWindow::menu_actionEnglish_triggered()
{
    this->ui->actionEnglish->setChecked(   true );
    this->ui->actionEspanol->setChecked(  false );
    this->ui->actionFrancais->setChecked( false );
    this->ui->actionItaliano->setChecked( false );
    this->language = "en";
    this->updateUiLanguage();
}
void MainWindow::menu_actionEspanol_triggered()
{
    this->ui->actionEnglish->setChecked(  false );
    this->ui->actionEspanol->setChecked(   true );
    this->ui->actionFrancais->setChecked( false );
    this->ui->actionItaliano->setChecked( false );
    this->language = "es";
    this->updateUiLanguage();
}
void MainWindow::menu_actionFrancais_triggered()
{
    this->ui->actionEnglish->setChecked(  false );
    this->ui->actionEspanol->setChecked(  false );
    this->ui->actionFrancais->setChecked(  true );
    this->ui->actionItaliano->setChecked( false );
    this->language = "fr";
    this->updateUiLanguage();
}
void MainWindow::menu_actionItaliano_triggered()
{

    this->ui->actionEnglish->setChecked(  false );
    this->ui->actionEspanol->setChecked(  false );
    this->ui->actionFrancais->setChecked( false );
    this->ui->actionItaliano->setChecked(  true );
    this->language = "it";
    this->updateUiLanguage();
}

//////////////////
//// LANGUAGE ////
//////////////////
void MainWindow::updateUiLanguage()
{
    // remove the old translator
    QCoreApplication::removeTranslator( &this->translator );
    if ( this->translator.load( QString(":/translations/%1").arg(QString::fromStdString( this->language )) ) ) {
        // apply the new translator
        QCoreApplication::installTranslator( &this->translator );
        this->ui->retranslateUi( this );
    }
}



/////////////////
//// ACTIONS ////
/////////////////
void MainWindow::on_button_Next_clicked()
{
    switch ( this->step ) {
        case 1:
            this->step ++;
            this->ui->stacked_Pages->setCurrentIndex( 1 );
            this->ui->button_Next->setText( MainWindow::tr( "Install" ) );
            this->ui->button_Back->setVisible( true );
            this->ui->label_Install_Step1->setPalette( this->PALETTE_norm );
            this->ui->label_Install_Step2->setPalette( this->PALETTE_step );
            break;
        case 2:
            this->ui->stacked_Main->setCurrentIndex( 1 );
            this->startInstalling();
            break;
    }
}


void MainWindow::on_button_Back_clicked()
{
    switch ( this->step ) {
        case 1:
            // shouldn't be reachable
            break;
        case 2:
            this->step --;
            this->ui->stacked_Pages->setCurrentIndex( 0 );
            this->ui->button_Next->setText( MainWindow::tr( "Next" ) );
            this->ui->button_Back->setVisible( false );
            this->ui->label_Install_Step1->setPalette( this->PALETTE_step );
            this->ui->label_Install_Step2->setPalette( this->PALETTE_norm );
            break;
    }
}


void MainWindow::on_checkBox_MenuEntry_toggled(bool checked)
{
    this->make_menu_entry = checked;
}

void MainWindow::startInstalling()
{
    this->installing = true;
    delete this->waiter_timer;
    this->waiter_timer = new QTimer(this);
    connect(this->waiter_timer, SIGNAL(timeout()), this, SLOT(checkInstallProgress()));
    // worker
    delete this->installer_timer;
    this->installer_timer = new QTimer(this);
    this->installer_timer->setSingleShot( true );
    connect(this->installer_timer, SIGNAL(timeout()), this, SLOT(Install()));
    // run
    this->waiter_timer->start(250);
    this->installer_timer->start(250);
}

void MainWindow::checkInstallProgress()
{
    if ( !this->installing ) {
        this->waiter_timer->stop();
        this->ui->stacked_Main->setCurrentIndex( 2 );
    }
}

void MainWindow::Install()
{
    bool ok = true;
    std::error_code err;

    this->ui->progressBar_Install->setValue( 0 );
    this->ui->label_Install_Info->setText( MainWindow::tr( "Checking the executable path ..." ) );
    // check the executable path
    if ( ! std::filesystem::exists( this->exec_path ) ) {
        this->ui->progressBar_Install->setValue( 10 );
        // path does not exists
        if ( this->OS == 1 ) {
            // on unix. path is /usr/bin/
            ok = false;
            DialogMsg dialog = DialogMsg(
                MainWindow::tr( "Installation failed" ),
                QString("%1:\n%2").arg(
                    MainWindow::tr( "The path does not exist" ),
                    QString::fromStdString( this->exec_path.string() ) ),
                QString::fromStdString( err.message() ), 2, nullptr );
            std::ignore = dialog.exec();
        } else {
            // on windows/mac. must create the new folder
            ok = std::filesystem::create_directory( this->exec_path, err );
            if ( !ok ) {
                // failed to create
                DialogMsg dialog = DialogMsg(
                    MainWindow::tr( "Installation failed" ),
                    QString("%1:\n%2").arg(
                        MainWindow::tr( "Failed to create the directory" ),
                        QString::fromStdString( this->exec_path.string() ) ),
                    QString::fromStdString( err.message() ), 2, nullptr );
                std::ignore = dialog.exec();
            }
        }

    } else {
        this->ui->progressBar_Install->setValue( 5 );
        // path exists
        if ( this->OS == 1 ) {
            // on unix. check if the executable already exists
            const std::filesystem::path path = this->exec_path.string() + "/LogDoctor";
            if ( std::filesystem::exists( path ) ) {
                // an entry already exists, ask to overwrite it
                {
                    DialogBool dialog = DialogBool(
                        MainWindow::tr( "Conflict" ),
                        QString("%1:\n%2\n\n%3").arg(
                            (std::filesystem::is_regular_file( path ))
                                ? MainWindow::tr( "An executable already exists" )
                                : MainWindow::tr( "An entry with the same name already exists" ),
                            QString::fromStdString( path.string() ),
                            MainWindow::tr( "If you choose to proceed, it will be overwritten\nContinue?" ) ),
                        nullptr );
                    ok = dialog.exec();
                }
                if ( ok ) {
                    this->ui->progressBar_Install->setValue( 10 );
                    // agreed on overwriting the entry
                    std::ignore = std::filesystem::remove_all( path, err );
                    ok = ! std::filesystem::exists( path );
                    if ( !ok ) {
                        // failed to remove
                        DialogMsg dialog = DialogMsg(
                            MainWindow::tr( "Installation failed" ),
                            QString("%1:\n%2").arg(
                                MainWindow::tr( "Failed to remove the entry" ),
                                QString::fromStdString( this->exec_path.string() ) ),
                            QString::fromStdString( err.message() ), 2, nullptr );
                        std::ignore = dialog.exec();
                    }
                }
            }
        } else {
            // on windows/mac
            if ( ! std::filesystem::is_directory( this->exec_path ) ) {
                // not a directory, ask to overwrite it
                {
                    DialogBool dialog = DialogBool(
                        MainWindow::tr( "Conflict" ),
                        QString("%1:\n%2\n\n%3").arg(
                            MainWindow::tr( "An entry with the same name already exists" ),
                            QString::fromStdString( this->exec_path.string() ),
                            MainWindow::tr( "If you choose to proceed, it will be overwritten\nContinue?" ) ),
                        nullptr );
                    ok = dialog.exec();
                }
                if ( ok ) {
                    // agreed on overwriting the entry
                    ok = std::filesystem::remove( this->exec_path, err );
                    if ( !ok ) {
                        // failed to remove
                        DialogMsg dialog = DialogMsg(
                            MainWindow::tr( "Installation failed" ),
                            QString("%1:\n%2").arg(
                                MainWindow::tr( "Failed to remove the entry" ),
                                QString::fromStdString( this->exec_path.string() ) ),
                            QString::fromStdString( err.message() ), 2, nullptr );
                        std::ignore = dialog.exec();
                    } else {
                        this->ui->progressBar_Install->setValue( 10 );
                        // succesfully removed, now create the folder
                        ok = std::filesystem::create_directory( this->exec_path, err );
                        if ( !ok ) {
                            // failed to create
                            DialogMsg dialog = DialogMsg(
                                MainWindow::tr( "Installation failed" ),
                                QString("%1:\n%2").arg(
                                    MainWindow::tr( "Failed to create the directory" ),
                                    QString::fromStdString( this->exec_path.string() ) ),
                                QString::fromStdString( err.message() ), 2, nullptr );
                            std::ignore = dialog.exec();
                        }
                    }
                }
            } else {
                this->ui->progressBar_Install->setValue( 5 );
                // installation altready exists, check the executable
                std::string ext = (this->OS==2) ? ".exe" : "";
                const std::vector<std::string> names = {"/LogDoctor","/uninstall"};
                for ( const auto& name : names ) {
                    const std::filesystem::path path = this->exec_path.string() + name + ext;
                    if ( std::filesystem::exists( path ) ) {
                        // an entry already exists, ask to overwrite it
                        {
                            DialogBool dialog = DialogBool(
                                MainWindow::tr( "Conflict" ),
                                QString("%1:\n%2\n\n%3").arg(
                                    (std::filesystem::is_regular_file( path ))
                                        ? MainWindow::tr( "An executable already exists" )
                                        : MainWindow::tr( "An entry with the same name already exists" ),
                                    QString::fromStdString( path.string() ),
                                    MainWindow::tr( "If you choose to proceed, it will be overwritten\nContinue?" ) ),
                                nullptr );
                            ok = dialog.exec();
                        }
                        if ( ok ) {
                            // agreed on overwriting the entry
                            std::ignore = std::filesystem::remove_all( path, err );
                            ok = ! std::filesystem::exists( path );
                            if ( !ok ) {
                                // failed to remove
                                DialogMsg dialog = DialogMsg(
                                    MainWindow::tr( "Installation failed" ),
                                    QString("%1:\n%2").arg(
                                        MainWindow::tr( "Failed to remove the entry" ),
                                        QString::fromStdString( this->exec_path.string() ) ),
                                    QString::fromStdString( err.message() ), 2, nullptr );
                                std::ignore = dialog.exec();
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    if ( ok ) {
        this->ui->progressBar_Install->setValue( 15 );
        this->ui->label_Install_Info->setText( MainWindow::tr( "Checking the configuration path ..." ) );
        // check the configurations path
        if ( ! std::filesystem::exists( this->conf_path ) ) {
            this->ui->progressBar_Install->setValue( 25 );
            // path does not exists, create it
            ok = std::filesystem::create_directory( this->conf_path, err );
            if ( !ok ) {
                // failed to create
                DialogMsg dialog = DialogMsg(
                    MainWindow::tr( "Installation failed" ),
                    QString("%1:\n%2").arg(
                        MainWindow::tr( "Failed to create the directory" ),
                        QString::fromStdString( this->conf_path.string() ) ),
                    QString::fromStdString( err.message() ), 2, nullptr );
                std::ignore = dialog.exec();
            }

        } else {
            // path already exists
            if ( !std::filesystem::is_directory( this->conf_path ) ) {
                // not a directory, ask to overwrite it
                {
                    DialogBool dialog = DialogBool(
                        MainWindow::tr( "Conflict" ),
                        QString("%1:\n%2\n\n%3").arg(
                            MainWindow::tr( "An entry with the same name already exists" ),
                            QString::fromStdString( this->conf_path.string() ),
                            MainWindow::tr( "If you choose to proceed, it will be overwritten\nContinue?" ) ),
                        nullptr );
                    ok = dialog.exec();
                }
                if ( ok ) {
                    // agreed on overwriting the entry
                    ok = std::filesystem::remove( this->conf_path, err );
                    if ( !ok ) {
                        // failed to remove
                        DialogMsg dialog = DialogMsg(
                            MainWindow::tr( "Installation failed" ),
                            QString("%1:\n%2").arg(
                                MainWindow::tr( "Failed to remove the entry" ),
                                QString::fromStdString( this->conf_path.string() ) ),
                            QString::fromStdString( err.message() ), 2, nullptr );
                        std::ignore = dialog.exec();
                    } else {
                        this->ui->progressBar_Install->setValue( 30 );
                        // succesfully removed, now create a new one
                        ok = std::filesystem::create_directory( this->conf_path, err );
                        if ( !ok ) {
                            // failed to create
                            DialogMsg dialog = DialogMsg(
                                MainWindow::tr( "Installation failed" ),
                                QString("%1:\n%2").arg(
                                    MainWindow::tr( "Failed to create the directory" ),
                                    QString::fromStdString( this->conf_path.string() ) ),
                                QString::fromStdString( err.message() ), 2, nullptr );
                            std::ignore = dialog.exec();
                        }
                    }
                }
            } else {
                this->ui->progressBar_Install->setValue( 20 );
                // is a directory: probably an installation already exists, check if a cofiguration file is present
                const std::filesystem::path path = this->conf_path.string() + "/logdoctor.conf";
                if ( std::filesystem::exists( path ) ) {
                    // a configuration file already exists, ask to overwrite it or not
                    {
                        DialogBool dialog = DialogBool(
                            MainWindow::tr( "Conflict" ),
                            QString("%1:\n%2\n\n%3").arg(
                                (std::filesystem::is_regular_file( path ))
                                    ? MainWindow::tr( "An old configuration file already exists" )
                                    : MainWindow::tr( "An entry with the same name already exists" ),
                                QString::fromStdString( path.string() ),
                                MainWindow::tr( "It's suggested to renew it, but you can keep it by answering 'No'\nOverwrite the file?" ) ),
                            nullptr );
                        const bool choice = dialog.exec();
                        if ( choice ) {
                            this->overwrite_conf_file = true;
                        }
                    }
                    if ( this->overwrite_conf_file ) {
                        this->ui->progressBar_Install->setValue( 25 );
                        // agreed on overwriting the entry
                        std::ignore = std::filesystem::remove_all( path, err );
                        ok = ! std::filesystem::exists( path );
                        if ( !ok ) {
                            // failed to remove
                            DialogMsg dialog = DialogMsg(
                                MainWindow::tr( "Installation failed" ),
                                QString("%1:\n%2").arg(
                                    MainWindow::tr( "Failed to remove the entry" ),
                                    QString::fromStdString( this->exec_path.string() ) ),
                                QString::fromStdString( err.message() ), 2, nullptr );
                            std::ignore = dialog.exec();
                        }
                    }
                } else {
                    // place the new conf file
                    this->overwrite_conf_file = true;
                }
            }
        }
    }

    if ( ok ) {
        this->ui->progressBar_Install->setValue( 30 );
        this->ui->label_Install_Info->setText( MainWindow::tr( "Checking the application data path ..." ) );
        // check the application data path
        if ( !std::filesystem::exists( this->data_path ) ) {
            this->ui->progressBar_Install->setValue( 40 );
            // path does not exists, create it
            ok = std::filesystem::create_directory( this->data_path, err );
            if ( !ok ) {
                // failed to create
                DialogMsg dialog = DialogMsg(
                    MainWindow::tr( "Installation failed" ),
                    QString("%1:\n%2").arg(
                        MainWindow::tr( "Failed to create the directory" ),
                        QString::fromStdString( this->data_path.string() ) ),
                    QString::fromStdString( err.message() ), 2, nullptr );
                std::ignore = dialog.exec();
            }

        } else {
            // path already exists
            if ( !std::filesystem::is_directory( this->data_path ) ) {
                // not a directory
                {
                    DialogBool dialog = DialogBool(
                        MainWindow::tr( "Conflict" ),
                        QString("%1:\n%2\n\n%3").arg(
                            MainWindow::tr( "An entry with the same name already exists" ),
                            QString::fromStdString( this->data_path.string() ),
                            MainWindow::tr( "If you choose to proceed, it will be overwritten\nContinue?" ) ),
                        nullptr );
                    ok = dialog.exec();
                }
                if ( ok ) {
                    // agreed on overwriting the entry
                    std::ignore = std::filesystem::remove_all( this->data_path, err );
                    ok = ! std::filesystem::exists( this->data_path );
                    if ( !ok ) {
                        // failed to remove
                        DialogMsg dialog = DialogMsg(
                            MainWindow::tr( "Installation failed" ),
                            QString("%1:\n%2").arg(
                                MainWindow::tr( "Failed to remove the entry" ),
                                QString::fromStdString( this->data_path.string() ) ),
                            QString::fromStdString( err.message() ), 2, nullptr );
                        std::ignore = dialog.exec();
                    } else {
                        this->ui->progressBar_Install->setValue( 40 );
                        // succesfully removed, now create a new one
                        ok = std::filesystem::create_directory( this->data_path, err );
                        if ( !ok ) {
                            // failed to create
                            DialogMsg dialog = DialogMsg(
                                MainWindow::tr( "Installation failed" ),
                                QString("%1:\n%2").arg(
                                    MainWindow::tr( "Failed to create the directory" ),
                                    QString::fromStdString( this->data_path.string() ) ),
                                QString::fromStdString( err.message() ), 2, nullptr );
                            std::ignore = dialog.exec();
                        }
                    }
                }
            } else {
                // is a directory: probably an installation already exists
                {
                    DialogBool dialog = DialogBool(
                        MainWindow::tr( "Conflict" ),
                        QString("%1:\n%2\n\n%3").arg(
                            MainWindow::tr( "A directory already exists for the application data" ),
                            QString::fromStdString( this->data_path.string() ),
                            MainWindow::tr( "If you choose to proceed, the content will be overwritten,\nexception made for databases, which won't be affected\nContinue?" ) ),
                        nullptr );
                    ok = dialog.exec();
                }
                if ( ok ) {
                    this->ui->progressBar_Install->setValue( 40 );
                    // agreed on overwriting content, remove it
                    std::vector<std::filesystem::path> paths = {
                        this->data_path.string() + "/help" };
                    if ( this->OS != 3 ) { // mac .app already contains it
                        paths.push_back( this->data_path.string() + "/licenses" );
                    }
                    for ( const auto& path : paths ) {
                        // remove the entries
                        if ( !std::filesystem::exists( path ) ) {
                            std::ignore = std::filesystem::remove_all( path, err );
                            ok = ! std::filesystem::exists( path );
                            if ( !ok ) {
                                // failed to remove
                                DialogMsg dialog = DialogMsg(
                                    MainWindow::tr( "Installation failed" ),
                                    QString("%1:\n%2").arg(
                                        MainWindow::tr( "Failed to remove the entry" ),
                                        QString::fromStdString( this->data_path.string() ) ),
                                    QString::fromStdString( err.message() ), 2, nullptr );
                                std::ignore = dialog.exec();
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    // COPY //

    // if everything went fine, start moving the content
    if ( ok ) {
        this->ui->progressBar_Install->setValue( 50 );
        this->ui->label_Install_Info->setText( MainWindow::tr( "Copying the executable file ..." ) );
        // move the executable
        std::string exec_name;
        switch ( this->OS ) {
            case 1:
                exec_name = "logdoctor"; break;
            case 2:
                exec_name = "LogDoctor.exe"; break;
            case 3:
                exec_name = "LogDoctor.app"; break;
            default:
                throw( "Unexpected OS: "[this->OS] );
        }
        const std::filesystem::path path = this->exec_path.string()+"/"+exec_name;
        ok = std::filesystem::copy_file( exec_name, path, std::filesystem::copy_options::overwrite_existing, err );
        // set permission
        if ( !ok ) {
            // failed to copy
            DialogMsg dialog = DialogMsg(
                MainWindow::tr( "Installation failed" ),
                QString("%1:\n%2").arg(
                    MainWindow::tr( "Failed to copy the resource" ),
                    QString::fromStdString( path.string() ) ),
                QString::fromStdString( err.message() ), 2, nullptr );
            std::ignore = dialog.exec();

        } else {
            this->ui->progressBar_Install->setValue( 55 );
            try {
                std::filesystem::permissions( path, std::filesystem::perms::owner_all, std::filesystem::perm_options::add );
                std::filesystem::permissions( path, std::filesystem::perms::group_all, std::filesystem::perm_options::remove );
                std::filesystem::permissions( path, std::filesystem::perms::group_read, std::filesystem::perm_options::add );
                std::filesystem::permissions( path, std::filesystem::perms::others_all, std::filesystem::perm_options::remove );
                switch ( this->OS ) {
                    case 1:
                        // 7 5 5
                        std::filesystem::permissions( path, std::filesystem::perms::others_exec, std::filesystem::perm_options::add );
                    case 3:
                        // 7 5 4
                        std::filesystem::permissions( path, std::filesystem::perms::group_exec, std::filesystem::perm_options::add );
                        std::filesystem::permissions( path, std::filesystem::perms::others_read, std::filesystem::perm_options::add );
                        break;
                    case 2:
                        // rw r -
                        break;
                    default:
                        throw( "Unexpected OS: "[this->OS] );
                }
            } catch (...) {
                ok = false;
                // failed set permissions
                DialogMsg dialog = DialogMsg(
                    MainWindow::tr( "Installation failed" ),
                    QString("%1:\n%2").arg(
                        MainWindow::tr( "Failed to assign permissions to the resource" ),
                        QString::fromStdString( path.string() ) ),
                    "", 2, nullptr );
                std::ignore = dialog.exec();
            }
        }


        if ( ok ) {
            this->ui->progressBar_Install->setValue( 60 );
            this->ui->label_Install_Info->setText( MainWindow::tr( "Copying the configuration file ..." ) );
            // continue moving stuff: now the config file
            if ( this->overwrite_conf_file ) {
                // no previous config file found or choosed to replace it
                const std::filesystem::path path_ = this->conf_path.string()+"/logdoctor.conf";
                ok = std::filesystem::copy_file( "logdoctor.conf", path_, std::filesystem::copy_options::overwrite_existing, err );
                if ( !ok ) {
                    // failed to move
                    DialogMsg dialog = DialogMsg(
                        MainWindow::tr( "Installation failed" ),
                        QString("%1:\n%2").arg(
                            MainWindow::tr( "Failed to copy the resource" ),
                            QString::fromStdString( path_.string() ) ),
                        QString::fromStdString( err.message() ), 2, nullptr );
                    std::ignore = dialog.exec();
                }
            }

            if ( ok ) {
                this->ui->progressBar_Install->setValue( 65 );
                this->ui->label_Install_Info->setText( MainWindow::tr( "Copying the application resources ..." ) );
                // continue moving stuff: now the resources
                const std::vector<std::string> names = { "licenses", "help" };
                for ( const auto& name : names ) {
                    // remove the entries
                    const std::filesystem::path path_ = this->data_path.string()+"/"+name;
                    std::filesystem::rename( name, path_, err );
                    if ( err.value() != 0 ) {
                        // failed to move
                        ok = false;
                        DialogMsg dialog = DialogMsg(
                            MainWindow::tr( "Installation failed" ),
                            QString("%1:\n%2").arg(
                                MainWindow::tr( "Failed to copy the resource" ),
                                QString::fromStdString( path_.string() ) ),
                            QString::fromStdString( err.message() ), 2, nullptr );
                        std::ignore = dialog.exec();
                        break;
                    }
                }
            }
        }

        if ( ok && this->OS != 3 ) { // mac .app contains it
            this->ui->progressBar_Install->setValue( 85 );
            this->ui->label_Install_Info->setText( MainWindow::tr( "Copying the uninstaller ..." ) );
            // move the uninstaller
            std::filesystem::path path_;
            switch ( this->OS ) {
                case 1:
                    path_ = this->data_path.string()+"/uninstall";
                    break;
                case 2:
                    path_ = this->exec_path.string()+"/uninstall.exe";
                    break;
                default:
                    throw( "Unexpected OS: "[this->OS] );
            }
            ok = std::filesystem::copy_file( exec_name, path_, std::filesystem::copy_options::overwrite_existing, err );
            // set permission
            if ( !ok ) {
                // failed to copy
                DialogMsg dialog = DialogMsg(
                    MainWindow::tr( "Installation failed" ),
                    QString("%1:\n%2").arg(
                        MainWindow::tr( "Failed to copy the resource" ),
                        QString::fromStdString( path_.string() ) ),
                    QString::fromStdString( err.message() ), 2, nullptr );
                std::ignore = dialog.exec();

            } else {
                try {
                    std::filesystem::permissions( path_, std::filesystem::perms::owner_all, std::filesystem::perm_options::add );
                    std::filesystem::permissions( path_, std::filesystem::perms::group_all, std::filesystem::perm_options::remove );
                    std::filesystem::permissions( path_, std::filesystem::perms::group_read, std::filesystem::perm_options::add );
                    std::filesystem::permissions( path_, std::filesystem::perms::others_all, std::filesystem::perm_options::remove );
                    switch ( this->OS ) {
                        case 1:
                            // 7 5 4
                        std::filesystem::permissions( path_, std::filesystem::perms::group_exec, std::filesystem::perm_options::add );
                        std::filesystem::permissions( path_, std::filesystem::perms::others_read, std::filesystem::perm_options::add );
                            break;
                        case 2:
                            // rw r -
                            break;
                        default:
                            throw( "Unexpected OS: "[this->OS] );
                    }
                } catch (...) {
                    ok = false;
                    // failed set permissions
                    DialogMsg dialog = DialogMsg(
                        MainWindow::tr( "Installation failed" ),
                        QString("%1:\n%2").arg(
                            MainWindow::tr( "Failed to assign permissions to the resource" ),
                            QString::fromStdString( path_.string() ) ),
                        "", 2, nullptr );
                    std::ignore = dialog.exec();
                }
            }
        }
    }

    if ( ok && this->OS != 3 ) { // mac .app contains these
        this->ui->progressBar_Install->setValue( 90 );
        this->ui->label_Install_Info->setText( MainWindow::tr( "Copying the icon ..." ) );
        // move the icon
        std::filesystem::path path;
        switch ( this->OS ) {
            case 1:
                // unix
                path = "/usr/share/icons/logdoctor.svg";
                break;
            case 2:
                path = this->exec_path.string() + "/LogDoctor.svg";
                break;
            default:
                throw( "Unexpected OS: "[this->OS] );
        }
        ok = std::filesystem::copy_file( "logdoctor.svg", path, std::filesystem::copy_options::overwrite_existing, err );
        if ( !ok ) {
            // failed to move
            DialogMsg dialog = DialogMsg(
                MainWindow::tr( "Installation failed" ),
                QString("%1:\n%2").arg(
                    MainWindow::tr( "Failed to copy the resource" ),
                    QString::fromStdString( path.string() ) ),
                QString::fromStdString( err.message() ), 2, nullptr );
            std::ignore = dialog.exec();
        }


        // make the menu entry
        if ( ok && this->make_menu_entry ) {
            this->ui->progressBar_Install->setValue( 95 );
            this->ui->label_Install_Info->setText( MainWindow::tr( "Creating the menu entry ..." ) );
            std::filesystem::path p;
            switch ( this->OS ) {
                case 1:
                    // unix
                    p = this->home_path+"/.local/share/applications/LogDoctor.desktop";
                    std::filesystem::rename( "LogDoctor.desktop", p, err );
                    ok = std::filesystem::exists( p );
                    if ( !ok ) {
                        // failed to move
                        DialogMsg dialog = DialogMsg(
                            MainWindow::tr( "Error" ),
                            QString("%1:\n%2").arg(
                                MainWindow::tr( "Failed to create the menu entry" ),
                                QString::fromStdString( p.string() ) ),
                            QString::fromStdString( err.message() ), 1, nullptr );
                        std::ignore = dialog.exec();
                    }
                    break;
                case 2:
                    p = this->home_path.substr(0,2) + "/ProgramData/Microsoft/Windows/Start Menu/Programs/LogDoctor.exe";
                    if ( std::filesystem::exists( p ) ) {
                        // an old entry already exists, remove it
                        std::ignore = std::filesystem::remove( p, err );
                        ok = ! std::filesystem::exists( p );
                    }
                    if ( ok ) {
                        std::filesystem::create_symlink( this->exec_path.string()+"/LogDoctor.exe", p, err );
                        if ( !std::filesystem::exists( p ) ) {
                            // failed to create
                            ok = false;
                        }
                    }
                    if ( !ok ) {
                        DialogMsg dialog = DialogMsg(
                            MainWindow::tr( "Error" ),
                            QString("%1:\n%2").arg(
                                MainWindow::tr( "Failed to create the menu entry" ),
                                QString::fromStdString( p.string() ) ),
                            QString::fromStdString( err.message() ), 1, nullptr );
                        std::ignore = dialog.exec();
                    }
                    break;
                case 3:
                    // still have to understand apple
                    break;
            }
        }
    }

    // proocess finished
    if ( ok ) {
        this->ui->progressBar_Install->setValue( 100 );
        this->ui->label_Install_Info->setText( MainWindow::tr( "Final steps ..." ) );
        // succesfully
        this->ui->label_Done_Status->setText( MainWindow::tr( "Installation successful" ) );
    } else {
        // with a failure
        this->ui->label_Done_Status->setText( MainWindow::tr( "Installation failed" ) );
        this->ui->label_Done_Info->setText( MainWindow::tr( "An error occured while installing LogDoctor" ) );
    }
    this->installing = false;
}



void MainWindow::on_button_Close_clicked()
{
    this->close();
}
