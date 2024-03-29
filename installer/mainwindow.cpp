
#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "modules/dialogs/dialogmsg.h"
#include "modules/dialogs/dialogbool.h"

#include <QFontDatabase>

#if defined( Q_OS_LINUX ) || defined( Q_OS_BSD4 )// || defined( Q_OS_MACOS )
    #include <unistd.h>
    #include <pwd.h>
#endif


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->ui->setupUi(this);
    #if defined( Q_OS_MACOS )
        this->ui->checkBox_MenuEntry->setChecked(false);
        this->ui->checkBox_MenuEntry->setCheckable(false);
        this->ui->checkBox_MenuEntry->setEnabled(false);
    #endif

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

void MainWindow::on_button_Close_clicked()
{
    this->close();
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
        default:
            throw( "Unexpected Installatio-Step: "[this->step] );
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
        default:
            throw( "Unexpected Installatio-Step: "[this->step] );
    }
}


void MainWindow::on_checkBox_MenuEntry_toggled(bool checked)
{
    this->make_menu_entry = checked;
}



//////////////////////
//// INSTALLATION ////
//////////////////////
void MainWindow::startInstalling()
{
    this->installing = true;
    this->waiter_timer.reset( new QTimer(this) );
    connect(this->waiter_timer.get(), &QTimer::timeout, this, &MainWindow::checkInstallProgress);
    // worker
    this->installer_timer.reset( new QTimer(this) );
    this->installer_timer->setSingleShot( true );
    connect(this->installer_timer.get(), &QTimer::timeout, this, &MainWindow::Install);
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
    bool ok{ true };

    this->ui->progressBar_Install->setValue( 0 );
    this->ui->label_Install_Info->setText( MainWindow::tr( "Checking the executable path ..." ) );
    // check the executable path
    ok = this->checkExecutablePath();

    if ( ok ) {
        this->ui->progressBar_Install->setValue( 15 );
        this->ui->label_Install_Info->setText( MainWindow::tr( "Checking the configuration path ..." ) );
        // check the configurations path
        ok = this->checkConfigsPath();
    }

    if ( ok ) {
        this->ui->progressBar_Install->setValue( 30 );
        this->ui->label_Install_Info->setText( MainWindow::tr( "Checking the application data path ..." ) );
        // check the application data path
        ok = this->checkAppdataPath();
    }

    // COPY //

    // if everything went fine, start moving the content
    if ( ok ) {
        this->ui->progressBar_Install->setValue( 50 );
        this->ui->label_Install_Info->setText( MainWindow::tr( "Copying the executable file ..." ) );
        // move the executable
        ok = this->copyExecutable();


        if ( ok ) {
            this->ui->progressBar_Install->setValue( 60 );
            // continue moving stuff: now the config file
            if ( this->overwrite_conf_file ) {
                this->ui->label_Install_Info->setText( MainWindow::tr( "Copying the configuration file ..." ) );
                // no previous config file found or choosed to replace it
                ok = this->copyConfigfile();
            }

            if ( ok ) {
                this->ui->progressBar_Install->setValue( 65 );
                this->ui->label_Install_Info->setText( MainWindow::tr( "Copying the application resources ..." ) );
                // continue moving stuff: now the resources
                ok = this->copyResources();
            }
        }

        #if !defined( Q_OS_MACOS ) && (defined( Q_OS_LINUX ) || defined( Q_OS_BSD4 ))
            if ( ok ) { // OSX app bundle contains it, Windows copyed it already
                this->ui->progressBar_Install->setValue( 85 );
                this->ui->label_Install_Info->setText( MainWindow::tr( "Copying the uninstaller ..." ) );
                // move the uninstaller
                ok = this->copyUninstaller();
            }
        #endif
    }

    #if !defined( Q_OS_MACOS )
        if ( ok ) { // mac .app contains these
            this->ui->progressBar_Install->setValue( 90 );
            this->ui->label_Install_Info->setText( MainWindow::tr( "Copying the icon ..." ) );
            // move the icon
            ok = this->copyIcon();

            // make the menu entry
            if ( ok && this->make_menu_entry ) {
                this->ui->progressBar_Install->setValue( 95 );
                this->ui->label_Install_Info->setText( MainWindow::tr( "Creating the menu entry ..." ) );
                ok = this->makeMenuEntry();
            }
        }
    #endif

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



///////////////////
//// FUNCTIONS ////
///////////////////
bool MainWindow::checkExecutablePath()
{
    bool ok{ true };
    std::error_code err;

    if ( ! std::filesystem::exists( this->exec_path ) ) {
        // base executable path does not exists
        ok = false;
        DialogMsg dialog(
            MainWindow::tr( "Installation failed" ),
            QString("%1:\n%2").arg(
                MainWindow::tr( "The path does not exist" ),
                QString::fromStdString( this->exec_path.string() ) ),
            QString::fromStdString( err.message() ), 2, nullptr );
        std::ignore = dialog.exec();

    } else {
        this->ui->progressBar_Install->setValue( 5 );
        // path exists
        #if !defined( Q_OS_MACOS ) && (defined( Q_OS_LINUX ) || defined( Q_OS_BSD4 ))
            // check if the executable already exists
            const std::filesystem::path path{ this->exec_path / "logdoctor" };
            if ( std::filesystem::exists( path ) ) {
                // an entry already exists, ask to overwrite it
                {
                    DialogBool dialog(
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
                        DialogMsg dialog(
                            MainWindow::tr( "Installation failed" ),
                            QString("%1:\n%2").arg(
                                MainWindow::tr( "Failed to remove the entry" ),
                                QString::fromStdString( path.string() ) ),
                            QString::fromStdString( err.message() ), 2, nullptr );
                        std::ignore = dialog.exec();
                    }
                }
            }
        #else
            // on windows/mac
            if ( ! std::filesystem::is_directory( this->exec_path ) ) {
                // not a directory, ask to overwrite it
                {
                    DialogBool dialog(
                        MainWindow::tr( "Conflict" ),
                        QString("%1:\n%2\n\n%3").arg(
                            MainWindow::tr( "An entry with the same name already exists, but it's not a directory" ),
                            QString::fromStdString( this->exec_path.string() ),
                            MainWindow::tr( "If you choose to proceed, it will be overwritten\nContinue?" ) ),
                        nullptr );
                    ok = dialog.exec();
                }
                if ( ok ) {
                    // agreed on overwriting the entry
                    std::ignore = std::filesystem::remove_all( this->exec_path, err );
                    ok = ! std::filesystem::exists( this->exec_path );
                    if ( !ok ) {
                        // failed to remove
                        DialogMsg dialog(
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
                            DialogMsg dialog(
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
                #if defined( Q_OS_WINDOWS )
                    const std::filesystem::path path{ this->exec_path / "LogDoctor" };
                #elif defined( Q_OS_MACOS )
                    const std::filesystem::path path{ this->exec_path / "LogDoctor.app" };
                #endif
                if ( std::filesystem::exists( path ) ) {
                    // an entry already exists, ask to overwrite it
                    {
                        DialogBool dialog(
                            MainWindow::tr( "Conflict" ),
                            QString("%1:\n%2\n\n%3").arg(
                                (std::filesystem::is_regular_file( path ))
                                    ? MainWindow::tr( "Installation path already exists" )
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
                            DialogMsg dialog(
                                MainWindow::tr( "Installation failed" ),
                                QString("%1:\n%2").arg(
                                    MainWindow::tr( "Failed to remove the entry" ),
                                    QString::fromStdString( path.string() ) ),
                                QString::fromStdString( err.message() ), 2, nullptr );
                            std::ignore = dialog.exec();
                        }
                    }
                }
            }
        #endif
    }
    return ok;
}


bool MainWindow::checkConfigsPath()
{
    bool ok{ true };
    std::error_code err;

    if ( ! std::filesystem::exists( this->conf_path ) ) {
        this->ui->progressBar_Install->setValue( 25 );
        // path does not exists, create it
        ok = std::filesystem::create_directory( this->conf_path, err );
        if ( ok ) {
            // will copy the new file
            this->overwrite_conf_file = true;
        } else {
            // failed to create
            DialogMsg dialog(
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
                DialogBool dialog(
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
                    DialogMsg dialog(
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
                        DialogMsg dialog(
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
            const std::filesystem::path path{ this->conf_path / "logdoctor.conf" };
            if ( std::filesystem::exists( path ) ) {
                // a configuration file already exists, ask to overwrite it or not
                {
                    DialogBool dialog(
                        MainWindow::tr( "Conflict" ),
                        QString("%1:\n%2\n\n%3").arg(
                            (std::filesystem::is_regular_file( path ))
                                ? MainWindow::tr( "An old configuration file already exists" )
                                : MainWindow::tr( "An entry with the same name already exists" ),
                            QString::fromStdString( path.string() ),
                            MainWindow::tr( "It's suggested to renew it, but you can keep it by answering 'No'\nOverwrite the file?" ) ),
                        nullptr );
                    this->overwrite_conf_file = dialog.exec();
                }
                if ( this->overwrite_conf_file ) {
                    this->ui->progressBar_Install->setValue( 25 );
                    // agreed on overwriting the entry
                    std::ignore = std::filesystem::remove_all( path, err );
                    ok = ! std::filesystem::exists( path );
                    if ( !ok ) {
                        // failed to remove
                        DialogMsg dialog(
                            MainWindow::tr( "Installation failed" ),
                            QString("%1:\n%2").arg(
                                MainWindow::tr( "Failed to remove the entry" ),
                                QString::fromStdString( path.string() ) ),
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
    return ok;
}


bool MainWindow::checkAppdataPath()
{
    bool ok{ true };
    std::error_code err;

    if ( !std::filesystem::exists( this->data_path ) ) {
        this->ui->progressBar_Install->setValue( 40 );
        // path does not exists, create it
        ok = std::filesystem::create_directory( this->data_path, err );
        if ( !ok ) {
            // failed to create
            DialogMsg dialog(
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
                DialogBool dialog(
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
                    DialogMsg dialog(
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
                        DialogMsg dialog(
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
            #if defined( Q_OS_MACOS )
                const std::vector<std::filesystem::path> paths{
                    this->data_path / "help" };
            #else
                const std::vector<std::filesystem::path> paths{
                    this->data_path / "help",
                    this->data_path / "licenses" }; // OSX app bundle already contains it
            #endif
            // check if the data path contains any of the entries
            if ( std::any_of( paths.cbegin(), paths.cend(), [](const auto& p){return std::filesystem::exists(p);} ) ) {
                DialogBool dialog(
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
                for ( const auto& path : paths ) {
                    // remove the entries
                    if ( std::filesystem::exists( path ) ) {
                        std::ignore = std::filesystem::remove_all( path, err );
                        ok = ! std::filesystem::exists( path );
                        if ( !ok ) {
                            // failed to remove
                            DialogMsg dialog(
                                MainWindow::tr( "Installation failed" ),
                                QString("%1:\n%2").arg(
                                    MainWindow::tr( "Failed to remove the entry" ),
                                    QString::fromStdString( path.string() ) ),
                                QString::fromStdString( err.message() ), 2, nullptr );
                            std::ignore = dialog.exec();
                            break;
                        }
                    }
                }
            }
        }
    }
    return ok;
}


bool MainWindow::copyExecutable()
{
    bool ok{ true };
    std::error_code err;

    #if defined( Q_OS_MACOS )
        const std::filesystem::path src_path{ std::filesystem::canonical("./LogDoctor.app") };
        const std::filesystem::path dst_path{ this->exec_path / "LogDoctor.app" };
    #elif defined( Q_OS_WINDOWS )
        const std::filesystem::path src_path{ std::filesystem::canonical("../LogDoctor").make_preferred() };
        const std::filesystem::path dst_path{ this->exec_path / "LogDoctor" };
    #elif defined( Q_OS_LINUX ) || defined( Q_OS_BSD4 )
        const std::filesystem::path src_path{ "logdoctor" };
        const std::filesystem::path dst_path{ this->exec_path / "logdoctor" };
    #endif

    #if defined( Q_OS_MACOS ) || defined( Q_OS_WINDOWS )
        std::filesystem::copy( src_path, dst_path, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive, err );
        if ( err ) {
            ok = false;
        }
        #if defined( Q_OS_WINDOWS )
            else {
                // remove the installer
                std::error_code _;
                const std::filesystem::path installer_path{ dst_path / "install.exe" };
                std::ignore = std::filesystem::remove( installer_path, _ ); // no need to abort the installation in case of failure
            }
        #endif
    #else
        ok = std::filesystem::copy_file( src_path, dst_path, std::filesystem::copy_options::overwrite_existing, err );
    #endif

    if ( !ok ) {
        // failed to copy
        DialogMsg dialog(
            MainWindow::tr( "Installation failed" ),
            QString("%1:\n%2\n\n%3:\n%4").arg(
                MainWindow::tr( "Failed to copy the resource" ),
                QString::fromStdString( src_path.string() ),
                MainWindow::tr( "To destination path" ),
                QString::fromStdString( dst_path.string() ) ),
            QString::fromStdString( err.message() ), 2, nullptr );
        std::ignore = dialog.exec();

    } else {
        // set permissions
        this->ui->progressBar_Install->setValue( 55 );
        try {
            std::filesystem::permissions( dst_path, std::filesystem::perms::owner_all, std::filesystem::perm_options::add );
            std::filesystem::permissions( dst_path, std::filesystem::perms::group_all, std::filesystem::perm_options::remove );
            std::filesystem::permissions( dst_path, std::filesystem::perms::group_read, std::filesystem::perm_options::add );
            std::filesystem::permissions( dst_path, std::filesystem::perms::others_all, std::filesystem::perm_options::remove );
            #if defined( Q_OS_MACOS )
                // 7 5 4
                std::filesystem::permissions( dst_path, std::filesystem::perms::group_exec, std::filesystem::perm_options::add );
                std::filesystem::permissions( dst_path, std::filesystem::perms::others_read, std::filesystem::perm_options::add );
            #elif defined( Q_OS_LINUX ) || defined( Q_OS_BSD4 )
                // 7 5 5
                std::filesystem::permissions( dst_path, std::filesystem::perms::group_exec, std::filesystem::perm_options::add );
                std::filesystem::permissions( dst_path, std::filesystem::perms::others_read, std::filesystem::perm_options::add );
                std::filesystem::permissions( dst_path, std::filesystem::perms::others_exec, std::filesystem::perm_options::add );
            #elif defined( Q_OS_WINDOWS )
                // rw r -
            #endif

        } catch (...) {
            ok = false;
            // failed set permissions
            DialogMsg dialog(
                MainWindow::tr( "Installation failed" ),
                QString("%1:\n%2").arg(
                    MainWindow::tr( "Failed to assign permissions to the resource" ),
                    QString::fromStdString( dst_path.string() ) ),
                "", 2, nullptr );
            std::ignore = dialog.exec();
        }
    }
    return ok;
}


bool MainWindow::copyConfigfile()
{
    bool ok{ true };
    std::error_code err;

    #if defined( Q_OS_WINDOWS )
        const std::filesystem::path src_path{ std::filesystem::canonical("../logdoctor.conf").make_preferred() };
    #else
        const std::filesystem::path src_path{ std::filesystem::canonical("./logdoctor.conf") };
    #endif
    const std::filesystem::path dst_path{ this->conf_path / "logdoctor.conf" };

    ok = std::filesystem::copy_file( src_path, dst_path, std::filesystem::copy_options::overwrite_existing, err );
    if ( !ok ) {
        // failed to move
        DialogMsg dialog(
            MainWindow::tr( "Installation failed" ),
            QString("%1:\n%2\n\n%3:\n%4").arg(
                MainWindow::tr( "Failed to copy the resource" ),
                QString::fromStdString( src_path.string() ),
                MainWindow::tr( "To destination path" ),
                QString::fromStdString( dst_path.string() ) ),
            QString::fromStdString( err.message() ), 2, nullptr );
        std::ignore = dialog.exec();
    }
    #if defined( Q_OS_LINUX ) || defined( Q_OS_BSD4 )// || defined( Q_OS_MACOS )
        else {
            const passwd* p{ getpwnam(this->user_name.c_str()) };
            if ( chown( dst_path.c_str(), p->pw_uid, p->pw_gid ) != 0 ) {
                ok = false;
                DialogMsg dialog(
                    MainWindow::tr( "Installation failed" ),
                    QString("%1:\n%2").arg(
                        MainWindow::tr( "Failed to change the owner of the resource" ),
                        QString::fromStdString( dst_path.string() ) ),
                    QString::fromStdString( err.message() ), 2, nullptr );
                std::ignore = dialog.exec();
            }
        }
    #endif
    return ok;
}


bool MainWindow::copyResources()
{
    bool ok{ true };
    std::error_code err;

    #if defined( Q_OS_MACOS )
        const std::vector<std::string> names{
            "help" };
    #else
        const std::vector<std::string> names{
            "help",
            "licenses" }; // mac .app already contains it
    #endif
    for ( const auto& name : names ) {
        // remove the entries
        #if defined( Q_OS_WINDOWS )
        const std::filesystem::path src_path{ std::filesystem::canonical("../logdocdata").make_preferred() / name };
        #else
            const std::filesystem::path src_path{ std::filesystem::canonical("logdocdata") / name };
        #endif
        const std::filesystem::path dst_path{ this->data_path / name };
        std::filesystem::copy( src_path, dst_path, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive, err );
        if ( err.value() != 0 ) {
            // failed to move
            ok = false;
            DialogMsg dialog(
                MainWindow::tr( "Installation failed" ),
                QString("%1:\n%2\n\n%3:\n%4").arg(
                    MainWindow::tr( "Failed to copy the resource" ),
                    QString::fromStdString( src_path.string() ),
                    MainWindow::tr( "To destination path" ),
                    QString::fromStdString( dst_path.string() ) ),
                QString::fromStdString( err.message() ), 2, nullptr );
            std::ignore = dialog.exec();
            break;
        }
    }
    return ok;
}


#if !defined( Q_OS_MACOS )
#if defined( Q_OS_LINUX ) || defined( Q_OS_BSD4 )
bool MainWindow::copyUninstaller()
{
    bool ok{ true };
    std::error_code err;

    const std::filesystem::path src_path{ "uninstall" };
    const std::filesystem::path dst_path{ this->data_path / "uninstall" };

    ok = std::filesystem::copy_file( src_path, dst_path, std::filesystem::copy_options::overwrite_existing, err );
    if ( !ok ) {
        // failed to copy
        DialogMsg dialog(
            MainWindow::tr( "Installation failed" ),
            QString("%1:\n%2\n\n%3:\n%4").arg(
                MainWindow::tr( "Failed to copy the resource" ),
                QString::fromStdString( src_path.string() ),
                MainWindow::tr( "To destination path" ),
                QString::fromStdString( dst_path.string() ) ),
            QString::fromStdString( err.message() ), 2, nullptr );
        std::ignore = dialog.exec();

    } else {
        // set permissions
        try {
            // 7 5 5
            std::filesystem::permissions( dst_path, std::filesystem::perms::owner_all, std::filesystem::perm_options::add );
            std::filesystem::permissions( dst_path, std::filesystem::perms::group_all, std::filesystem::perm_options::remove );
            std::filesystem::permissions( dst_path, std::filesystem::perms::group_read, std::filesystem::perm_options::add );
            std::filesystem::permissions( dst_path, std::filesystem::perms::others_all, std::filesystem::perm_options::remove );
            std::filesystem::permissions( dst_path, std::filesystem::perms::group_exec, std::filesystem::perm_options::add );
            std::filesystem::permissions( dst_path, std::filesystem::perms::others_read, std::filesystem::perm_options::add );
            std::filesystem::permissions( dst_path, std::filesystem::perms::others_exec, std::filesystem::perm_options::add );

        } catch (...) {
            ok = false;
            // failed set permissions
            DialogMsg dialog(
                MainWindow::tr( "Installation failed" ),
                QString("%1:\n%2").arg(
                    MainWindow::tr( "Failed to assign permissions to the resource" ),
                    QString::fromStdString( dst_path.string() ) ),
                "", 2, nullptr );
            std::ignore = dialog.exec();
        }
    }
    return ok;
}
#endif // defined( Q_OS_LINUX ) || defined( Q_OS_BSD4 )


bool MainWindow::copyIcon()
{
    bool ok{ true };
    std::error_code err;

    #if defined( Q_OS_LINUX ) || defined( Q_OS_BSD4 )
        const std::filesystem::path src_path{ std::filesystem::canonical("LogDoctor.svg") };
        const std::filesystem::path dst_path{ "/usr/share/LogDoctor/LogDoctor.svg" };
    #elif defined( Q_OS_WINDOWS )
        const std::filesystem::path src_path{ std::filesystem::canonical("../LogDoctor.svg").make_preferred() };
        const std::filesystem::path dst_path{ this->exec_path / "LogDoctor" / "LogDoctor.svg" };
    #else
        throw( "LogDoctor: copyIcon(): Unexpected OS" );
    #endif

    ok = std::filesystem::copy_file( src_path, dst_path, std::filesystem::copy_options::overwrite_existing, err );
    if ( !ok ) {
        // failed
        DialogMsg dialog(
            MainWindow::tr( "Installation failed" ),
            QString("%1:\n%2\n\n%3:\n%4").arg(
                MainWindow::tr( "Failed to copy the resource" ),
                QString::fromStdString( src_path.string() ),
                MainWindow::tr( "To destination path" ),
                QString::fromStdString( dst_path.string() ) ),
            QString::fromStdString( err.message() ), 2, nullptr );
        std::ignore = dialog.exec();
    }
    return ok;
}


bool MainWindow::makeMenuEntry()
{
    bool ok{ true };
    std::error_code err;

    #if defined( Q_OS_LINUX )
        const std::filesystem::path src_path{ "LogDoctor.desktop" };
        const std::filesystem::path dst_path{ "/usr/share/applications/LogDoctor.desktop" };
        ok = std::filesystem::copy_file( src_path, dst_path, std::filesystem::copy_options::overwrite_existing, err );
    #elif defined( Q_OS_BSD4 )
        const std::filesystem::path src_path{ "LogDoctor.desktop" };
        const std::filesystem::path dst_path{ "/usr/local/share/applications/LogDoctor.desktop" };
        ok = std::filesystem::copy_file( src_path, dst_path, std::filesystem::copy_options::overwrite_existing, err );
    #elif defined( Q_OS_WINDOWS )
        const std::filesystem::path src_path{ this->exec_path / "LogDoctor" / "LogDoctor.exe" };
        const std::filesystem::path dst_path{ std::filesystem::path{this->home_path.substr(0,2) + "/ProgramData/Microsoft/Windows/Start Menu/Programs/LogDoctor.exe"}.make_preferred() };
        if ( std::filesystem::exists( dst_path ) ) {
            // an old entry already exists, remove it
            std::ignore = std::filesystem::remove( dst_path, err );
            ok = ! std::filesystem::exists( dst_path );
        }
        if ( ok ) {
            std::filesystem::create_symlink( src_path, dst_path, err );
            if ( !std::filesystem::exists( dst_path ) ) {
                // failed to create
                ok = false;
            }
        }
    #else
        throw( "LogDoctor: makeMenuEntry(): Unexpected OS" );
    #endif

    if ( !ok ) {
        // failed
        DialogMsg dialog(
            MainWindow::tr( "Error" ),
            QString("%1:\n%2").arg(
                MainWindow::tr( "Failed to create the menu entry" ),
                QString::fromStdString( dst_path.string() ) ),
            QString::fromStdString( err.message() ), 1, nullptr );
        std::ignore = dialog.exec();
    }
    return ok;
}
#endif
