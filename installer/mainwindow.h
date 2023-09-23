#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTranslator>
#include <QStandardPaths>
#include <QString>
#include <QTimer>

#include <filesystem>
#include <string>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:

    // language

    void menu_actionEnglish_triggered();

    void menu_actionEspanol_triggered();

    void menu_actionFrancais_triggered();

    void menu_actionItaliano_triggered();

    // installation

    void Install();

    void checkInstallProgress();


    // actions

    void on_button_Next_clicked();

    void on_button_Back_clicked();

    void on_checkBox_MenuEntry_toggled(bool checked);

    void on_button_Close_clicked();

private:
    QSharedPointer<Ui::MainWindow> ui;

    // operating system
    const std::string cleanPath( const QString& path );

    const std::string home_path{ this->cleanPath( QStandardPaths::locate( QStandardPaths::HomeLocation, "", QStandardPaths::LocateDirectory ) ) };
    #if defined( Q_OS_MACOS )
        const std::filesystem::path exec_path{ "/Applications" };
        const std::filesystem::path conf_path{ home_path + "/Lybrary/Preferences/LogDoctor" };
        const std::filesystem::path data_path{ home_path + "/Lybrary/Application Support/LogDoctor" };
    #elif defined( Q_OS_WINDOWS )
        const std::filesystem::path exec_path{ std::filesystem::path{home_path.substr(0,2) + "/Program Files"}.make_preferred() };
        const std::filesystem::path conf_path{ std::filesystem::path{home_path + "/AppData/Local/LogDoctor"}.make_preferred() };
        const std::filesystem::path data_path{ std::filesystem::path{home_path + "/AppData/Local/LogDoctor"}.make_preferred() };
    #elif defined( Q_OS_LINUX ) || defined( Q_OS_BSD4 )
        const std::filesystem::path exec_path{ "/usr/bin" };
        const std::filesystem::path conf_path{ home_path + "/.config/LogDoctor" };
        const std::filesystem::path data_path{ "/usr/share/LogDoctor" };
    #else
        #error "System not supported"
    #endif

    // language
    QTranslator translator;
    std::string language{ "en" };
    void updateUiLanguage();

    // fonts
    QFont FONT_main;
    QFont FONT_bigger;
    QFont FONT_paths;

    // palettes
    QPalette PALETTE_norm;
    QPalette PALETTE_step;

    // work related
    int step;
    bool make_menu_entry{ true };
    // install
    QScopedPointer<QTimer> waiter_timer;
    QScopedPointer<QTimer> installer_timer;
    bool installing;
    bool overwrite_conf_file{ false };

    void startInstalling();
    bool checkExecutablePath();
    bool checkConfigsPath();
    bool checkAppdataPath();
    bool copyExecutable();
    bool copyConfigfile();
    bool copyResources();
    #if defined( Q_OS_LINUX ) || defined( Q_OS_BSD4 )
        bool copyUninstaller();
    #endif
    #if !defined( Q_OS_MACOS )
        bool copyIcon();
        bool makeMenuEntry();
    #endif
};

#endif // MAINWINDOW_H
