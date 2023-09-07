# CXX = g++
# CXXFLAGS = --std=c++17
# SRCS = serverB.cpp serverA.cpp serverM.cpp client.cpp helper_funcs.cpp
# OBJS = $(SRCS:.cpp=.o)
# EXECUTABLES = serverB serverA serverM client

# all: $(EXECUTABLES)

# $(EXECUTABLES): %: %.o helper_funcs.o
# 	$(CXX) $(CXXFLAGS) $^ -o $@

# %.o: %.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

# clean:
# 	rm -f $(OBJS) $(EXECUTABLES)

CXX = g++
CXXFLAGS = --std=c++17

all: serverB serverA serverM client

serverB: serverB.o helper_funcs.o
	$(CXX) $(CXXFLAGS) serverB.o helper_funcs.o -o serverB

serverA: serverA.o helper_funcs.o
	$(CXX) $(CXXFLAGS) serverA.o helper_funcs.o -o serverA

serverM: serverM.o helper_funcs.o
	$(CXX) $(CXXFLAGS) serverM.o helper_funcs.o -o serverM

client: client.o
	$(CXX) $(CXXFLAGS) client.o -o client

serverB.o: serverB.cpp helper_funcs.h
	$(CXX) $(CXXFLAGS) -c serverB.cpp -o serverB.o

serverA.o: serverA.cpp helper_funcs.h
	$(CXX) $(CXXFLAGS) -c serverA.cpp -o serverA.o

serverM.o: serverM.cpp helper_funcs.h
	$(CXX) $(CXXFLAGS) -c serverM.cpp -o serverM.o

client.o: client.cpp
	$(CXX) $(CXXFLAGS) -c client.cpp -o client.o

helper_funcs.o: helper_funcs.cpp helper_funcs.h
	$(CXX) $(CXXFLAGS) -c helper_funcs.cpp -o helper_funcs.o

clean:
	rm -f serverB serverA serverM client *.o
