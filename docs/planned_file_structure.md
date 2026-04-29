```
FRE6883_TeamProject/
├── CMakeLists.txt
├── Makefile
├── README.md
├── CODE_STYLE.md
├── data/
│   ├── Russell3000_components.csv
│   └── Russell3000_Earnings.csv
├── include/
│   ├── interfaces/
│   │   ├── IStockUniverseReader.h
│   │   ├── IStockUniverseWriter.h
│   │   ├── IHistoricalPriceFetcher.h
│   │   ├── IEventStudyEngine.h
│   │   └── IVisualizer.h
│   ├── entities/
│   │   ├── Stock.h
│   │   └── Matrix.h
│   ├── services/
│   │   ├── MarketManager.h
│   │   ├── DataFetcher.h
│   │   ├── CSVParser.h
│   │   ├── BootstrapEngine.h
│   │   ├── GnuplotVisualizer.h
│   │   └── AppCoordinator.h
│   └── ui/
│       └── MenuController.h
├── src/
│   ├── entities/
│   │   ├── Stock.cpp
│   │   └── Matrix.cpp
│   ├── services/
│   │   ├── MarketManager.cpp
│   │   ├── DataFetcher.cpp
│   │   ├── CSVParser.cpp
│   │   ├── BootstrapEngine.cpp
│   │   ├── GnuplotVisualizer.cpp
│   │   └── AppCoordinator.cpp
│   ├── ui/
│   │   └── MenuController.cpp
│   └── main.cpp
├── docs/
│   ├── UML_Class.pdf
│   ├── UML_Seq.pdf
│   └── planned_file_structure.md
├── tests/
│   ├── test_stock.cpp
│   ├── test_matrix.cpp
│   └── test_market_manager.cpp
└── presentation/
    └── TeamX_Final_Presentation.pptx
```