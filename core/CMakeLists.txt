add_library(PACECore INTERFACE)

target_sources(PACECore INTERFACE
    ATAController.cpp
    CPU.cpp
    FloppyController.cpp
    GamePort.cpp
    QEMUConfig.cpp
    System.cpp
    VGACard.cpp
)

target_include_directories(PACECore INTERFACE ${CMAKE_CURRENT_LIST_DIR})