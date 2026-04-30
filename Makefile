# =============================================================================
# DataFetcher module — add these lines to the team's shared Makefile
# =============================================================================

CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS  = -lcurl

# Add DataFetcher.o and ApiConfig.o to the team's full object list
OBJS = DataFetcher.o ApiConfig.o MarketManager.o CSVParser.o AppCoordinator.o \
       BootstrapEngine.o MenuController.o GnuplotVisualizer.o main.o # ... append other team members' .o files

# --- Main project target (team fills in their main entry point) ---
# TARGET = project
# $(TARGET): $(OBJS)
# 	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# DataFetcher depends on its own header and the interface it implements
DataFetcher.o: DataFetcher.cpp DataFetcher.h IHistoricalPriceFetcher.h
	$(CXX) $(CXXFLAGS) -c DataFetcher.cpp -o DataFetcher.o

# ApiConfig implementation must now be compiled separately (Rule 5 fix)
ApiConfig.o: ApiConfig.cpp ApiConfig.h
	$(CXX) $(CXXFLAGS) -c ApiConfig.cpp -o ApiConfig.o

# Standalone test binary — not part of the final submission
test_fetcher: test_fetcher.cpp DataFetcher.o ApiConfig.o
	$(CXX) $(CXXFLAGS) -o test_fetcher test_fetcher.cpp \
	    DataFetcher.o ApiConfig.o $(LDFLAGS)

clean:
	rm -f *.o test_fetcher
