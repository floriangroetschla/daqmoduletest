cmake_minimum_required(VERSION 3.12)
project(daqmoduletest VERSION 1.1.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()


find_package(ers REQUIRED)
find_package(rcif REQUIRED)
find_package(appfwk REQUIRED)
# add boost
FIND_PACKAGE(Boost COMPONENTS program_options system log log_setup iostreams REQUIRED)
set(BOOST_LIBS Boost::program_options Boost::log Boost::log_setup Boost::iostreams ${Boost_LIBRARIES})

daq_codegen(*.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2 )

##############################################################################
# Note that when the ers library is linked in, it uses a "::" because
# it's in an external package, not a local CMake target. The semantics
# are <namespace>::<shared library / executable>
#daq_add_library(IntPrinter.cpp LINK_LIBRARIES ers::ers)

##############################################################################
#daq_add_application( daqmoduletest_test_program toylibrary_test_program.cxx TEST LINK_LIBRARIES ${Boost_PROGRAM_OPTIONS_LIBRARY} daqmoduletest )

##############################################################################
#daq_add_unit_test(ValueWrapper_test)

##############################################################################

daq_add_plugin(RandomProducer duneDAQModule LINK_LIBRARIES appfwk::appfwk)
daq_add_plugin(Consumer duneDAQModule LINK_LIBRARIES appfwk::appfwk ${BOOST_LIBS})

#daq_add_library(../plugins/Consumer.cpp LINK_LIBRARIES ${BOOST_LIBS})


daq_install()



