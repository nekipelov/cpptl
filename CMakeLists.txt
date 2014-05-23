PROJECT(cpptl)
CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

FIND_PACKAGE(Boost COMPONENTS filesystem REQUIRED)
FIND_PACKAGE(Boost COMPONENTS system REQUIRED)
FIND_PACKAGE(Boost COMPONENTS unit_test_framework REQUIRED )

INCLUDE(cmake/CheckCXX11Features.cmake)

IF(MSVC)
    SET( CMAKE_CXX_FLAGS "-W3 -DWIN32 -EHsc -DBOOST_ALL_NO_LIB")
ELSEIF(CMAKE_COMPILER_IS_GNUCXX)
    IF(HAS_CXX11_LAMBDA)
        SET( CMAKE_CXX_FLAGS "-Wextra -Wall -std=c++0x")
    ELSE(HAS_CXX11_LAMBDA)
        SET( CMAKE_CXX_FLAGS "-Wextra -Wall")
    ENDIF(HAS_CXX11_LAMBDA)
ELSE(MSVC)
    SET( CMAKE_CXX_FLAGS "-Wextra -Wall")
ENDIF(MSVC)

SET (HEADERS
    value.h
    templateasttree.h
    template.h
    templateengine.h
    templatecontext.h
    buildinhelpers.h
    parser.h
    scanner.h
)

SET (SOURCES
    templateasttree.cpp
    template.cpp
    templateengine.cpp
    buildinhelpers.cpp
    value.cpp
    scanner.c
    parser.c
)

ADD_CUSTOM_COMMAND(
   SOURCE scanner.l
   COMMAND flex
   ARGS -d ${CMAKE_CURRENT_SOURCE_DIR}/scanner.l
   TARGET hts-test
   OUTPUTS scanner.h scanner.c)

ADD_CUSTOM_COMMAND(
   SOURCE parser.y
   COMMAND bison
   ARGS --debug --verbose ${CMAKE_CURRENT_SOURCE_DIR}/parser.y -o ${CMAKE_CURRENT_BINARY_DIR}/parser.c
   TARGET hts-test
   DEPENDS scanner.c scanner.h
   OUTPUTS parser.c parser.h)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

IF(HAS_CXX11_RAW_STRING)
    ADD_EXECUTABLE(cpptl-test template_test.cpp ${SOURCES} ${HEADERS})

    TARGET_LINK_LIBRARIES(cpptl-test
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    )
ENDIF(HAS_CXX11_RAW_STRING)

ADD_EXECUTABLE(value-test value_test.cpp value.cpp value.h)

TARGET_LINK_LIBRARIES(value-test
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

ADD_LIBRARY(cpptl ${SOURCES} ${HEADERS})
TARGET_LINK_LIBRARIES(cpptl
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
)

INSTALL(TARGETS cpptl DESTINATION lib)
INSTALL(FILES value.h template.h templateengine.h DESTINATION include/cpptl)
