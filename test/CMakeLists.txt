CMAKE_MINIMUM_REQUIRED(VERSION 3.21)
PROJECT(test_market)

FIND_PACKAGE(Boost 1.40 COMPONENTS system REQUIRED)
FIND_PACKAGE(Threads REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_BUILD_TYPE Debug)
ADD_COMPILE_DEFINITIONS(TEST)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1
)

FetchContent_MakeAvailable(Catch2)

ADD_EXECUTABLE(tests.out test_market.cpp 
               ../src/market.cpp ../src/market.h 
               ../src/offer.cpp ../src/offer.h 
               ../src/user_data.cpp ../src/user_data.h 
               ../src/deal.cpp ../src/deal.h)

TARGET_LINK_LIBRARIES(tests.out PRIVATE Catch2::Catch2WithMain)
