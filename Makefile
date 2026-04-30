CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS  = -lcurl

# team's full object list
OBJS = DataFetcher.o ApiConfig.o MarketManager.o CSVParser.o AppCoordinator.o \
       BootstrapEngine.o MenuController.o GnuplotVisualizer.o main.o

# main entry point
# TARGET = main
# $(TARGET): $(OBJS)
# 	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

DataFetcher.o: DataFetcher.cpp DataFetcher.h IHistoricalPriceFetcher.h
	$(CXX) $(CXXFLAGS) -c DataFetcher.cpp -o DataFetcher.o

ApiConfig.o: ApiConfig.cpp ApiConfig.h
	$(CXX) $(CXXFLAGS) -c ApiConfig.cpp -o ApiConfig.o

# Standalone test binary — not part of the final submission
test_fetcher: test_fetcher.cpp DataFetcher.o ApiConfig.o
	$(CXX) $(CXXFLAGS) -o test_fetcher test_fetcher.cpp \
	    DataFetcher.o ApiConfig.o $(LDFLAGS)

clean:
	rm -f *.o test_fetcher


