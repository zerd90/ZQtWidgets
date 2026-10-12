cmake_minimum_required(VERSION 3.20)

project(qt-custom-widgets)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

aux_source_directory(widgets WIDGET_SOURCES)

# Headers live outside the .cpp directory, so AUTOMOC only emits their moc
# files when they are listed as sources. Otherwise staticMetaObject and the
# vtable are never linked.
set(WIDGET_HEADERS
    include/AppCheckBox.h
    include/StepButtonSlider.h
    include/ToggleSwitch.h
    include/UniformComboBox.h
)

add_library(${PROJECT_NAME} STATIC
    ${WIDGET_SOURCES}
    ${WIDGET_HEADERS}
    qt_custom_widgets.qrc
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    Qt6::Widgets
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
)
