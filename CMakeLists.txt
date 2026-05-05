cmake_minimum_required(VERSION 3.19)
project(ArtMuseum LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)

qt_standard_project_setup()

set(CMAKE_AUTORCC ON) #this is what made the resource files work

qt_add_executable(ArtMuseum
    WIN32 MACOSX_BUNDLE

    main.cpp

    MainWindow.cpp
    MainWindow.h
    MainWindow.ui

    MuseumWidget.cpp
    MuseumWidget.h
    MuseumWidget.h
    resources.qrc
    rendering/Vertex.h
    geometry/GeometryBuilder.h
    geometry/GeometryBuilder.cpp
    scene/RoomBuilder.cpp
    scene/RoomBuilder.h
    scene/PaintingBuilder.h
    scene/PaintingBuilder.cpp
    assets/textures/WallTexture.png
    assets/textures/FloorTexture.jpg
)

target_link_libraries(ArtMuseum
    PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::OpenGL
        Qt::OpenGLWidgets
)

include(GNUInstallDirs)

install(TARGETS ArtMuseum
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ArtMuseum
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
