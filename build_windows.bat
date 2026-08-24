@echo off
REM Compiles the C++ project on Windows using g++.
g++ -std=c++17 src\main.cpp src\TextProc.cpp src\Metrics.cpp src\FlickrLoader.cpp src\Florence.cpp src\Downloader.cpp src\WebParser.cpp src\Evaluation.cpp src\SemanticSimilarity.cpp src\Coefficients.cpp src\ImagePurpose.cpp src\ErrorAnalysis.cpp src\Calibration.cpp src\StatsUtil.cpp -I include -o app.exe
REM Shows the result.
echo Build finished. If no errors are shown, run app.exe
