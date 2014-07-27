BOOSTINCLUDE = /home/shengg/opt/boost/include
BOOSTLIB = -L/home/shengg/opt/boost/lib/ -lboost_serialization -lboost_system -lboost_filesystem
all:
	#mpicxx -std=c++11 -I${BOOSTINCLUDE} ${BOOSTLIB} externalsort.cpp
	mpicxx -std=c++0x -I${BOOSTINCLUDE} ${BOOSTLIB} externalsort.cpp
