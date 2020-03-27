all : thtest asynctest 
thtest : thtest.cpp
	g++ -std=c++11 -Wall -pthread thtest.cpp -o thtest

asynctest : asynctest.cpp
	g++ -std=c++11 -Wall -pthread asynctest.cpp -o asynctest
clean :
	rm thtest asynctest
