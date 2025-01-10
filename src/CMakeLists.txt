qt_add_executable(kde-caps-lock-osd
    main.cpp
)

# Use the QtQuick module from Qt 6.
target_link_libraries(kde-caps-lock-osd PRIVATE Qt6::Quick)

# Install the executable
install(TARGETS kde-caps-lock-osd DESTINATION bin)
