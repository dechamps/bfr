g++ -o udp0 udp0.cpp \
    -I../BFR ../BFR/BFRParser.cpp ../BFR/BFRUDP.cpp ../BFR/BFRConsole.cpp ../BFR/BFRRegistration.cpp \
    -I../BACnet ../BACnet/*.cpp \
    -I../MinML ../MinML/*.cpp \
    $@
