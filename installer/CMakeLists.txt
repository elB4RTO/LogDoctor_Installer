cmake_minimum_required(VERSION 3.5)

project(LogDoctorInstaller VERSION 2.00 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W2)
    set(CMAKE_CXX_FLAGS_MINSIZEREL "/Os")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES
    translations/LogDoctorInstaller_en.ts
    translations/LogDoctorInstaller_es.ts
    translations/LogDoctorInstaller_fr.ts
    translations/LogDoctorInstaller_it.ts
)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    modules/dialogs/dialogmsg.ui
    modules/dialogs/dialogmsg.h
    modules/dialogs/dialogmsg.cpp
    modules/dialogs/dialogbool.ui
    modules/dialogs/dialogbool.h
    modules/dialogs/dialogbool.cpp

    resources/resources.qrc
    ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(LogDoctorInstaller
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET LogDoctorInstaller APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(LogDoctorInstaller SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(LogDoctorInstaller
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic -lgcc -lstdc++ -lwinpthread")
endif()

target_link_libraries(LogDoctorInstaller PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)

if(WIN32)
    set_target_properties(LogDoctorInstaller PROPERTIES
        WIN32_EXECUTABLE TRUE)
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(LogDoctorInstaller)
endif()
