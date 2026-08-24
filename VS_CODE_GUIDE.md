# Visual Studio Code User Guide

This file describes how to set up, compile and run the project in Visual Studio Code on a Windows computer.

Expected project path in this guide:

```text
M:\VS_Code_Projects\AIDACAT_ENG_v23
```

If the project is in a different folder, update the commands and the `.vscode/tasks.json` file's path.

---

## 0. What to install first

This section is for someone starting completely from scratch. Look up the installation details in the software itself or online - this is just a list of WHAT to look for.

1. **Visual Studio Code** - download and install it from the official VS Code website.
2. **Two VS Code extensions** - open `Extensions` in VS Code (the icon in the left sidebar that looks like four squares), search for and install:

   | Extension | Publisher | Why it's needed |
   |---|---|---|
   | `C/C++` | Microsoft | C++ syntax highlighting and IntelliSense (not required to compile, but very helpful) |
   | `Python` | Microsoft | Python syntax highlighting and assistance |

3. **A C++ compiler (g++)** - for example MSYS2 (includes `g++` and other tools) or MinGW-w64. After installing, make sure `g++` is on your Windows `PATH` (see the check in section 9.4).
4. **Python** (3.10 or newer) - installed so that the `python` command works in a terminal.

Once these four items are ready, continue with section 1.

---

## 1. Opening the project in VS Code

1. Open Visual Studio Code.
2. Choose `File -> Open Folder`.
3. Open this folder:

```text
M:\VS_Code_Projects\AIDACAT_ENG_v23
```

A correctly opened project root should show these files and folders:

```text
include
src
python
data
sample_images
output
.vscode
build_windows.bat
requirements.txt
README.md
OVERVIEW.md
```

Important: VS Code must be opened on the project's root folder, not on `src`, `python`, or any other subfolder.

---

## 2. Opening a terminal

In VS Code choose:

```text
Terminal -> New Terminal
```

Check where the terminal is:

```powershell
pwd
```

The correct result should be:

```text
M:\VS_Code_Projects\AIDACAT_ENG_v23
```

If the terminal is in a different folder, move to the project folder:

```powershell
cd "M:\VS_Code_Projects\AIDACAT_ENG_v23"
```

---

## 3. Creating the Python environment

A virtual environment is needed for the Florence-2 and semantic-similarity Python modules. It is not needed for `experiment` mode alone, but it is needed for `single`, `web`, and `calibrate` mode.

Create the environment:

```powershell
python -m venv test_env
```

You can then activate it:

```powershell
.\test_env\Scripts\activate
```

If PowerShell shows an error about blocked scripts, see the `Troubleshooting` section.

Important: you can also run the program without activating the environment, by giving the full Python path in the commands:

```text
M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe
```

This is exactly the approach used in `tasks.json`.

---

## 4. Installing Python libraries

If the environment is activated:

```powershell
python -m pip install --upgrade pip
pip install -r requirements.txt
```

If you don't activate it, use the full Python path:

```powershell
& "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe" -m pip install --upgrade pip
& "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe" -m pip install -r requirements.txt
```

**Important:** always install from `requirements.txt`, not library by library. The file pins exact, matching versions (`transformers==4.49.0` and `sentence-transformers==4.1.0`) - if you install `sentence-transformers` separately, `pip` may pull in a newer `transformers` that breaks Florence-2 (see section 4 of `USER_MANUAL.md`).

Check:

```powershell
& "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe" -c "import torch; print(torch.__version__); print(torch.cuda.is_available())"
```

If `torch.cuda.is_available()` shows `False`, that's not a defect. It just means Florence-2 will run in CPU mode (slower, but working).

---

## 5. Testing Florence-2 and semantic similarity without the C++ program

Before using `single`, `web`, or `calibrate` mode, test both Python scripts on their own.

Florence-2:

```powershell
& "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe" "M:\VS_Code_Projects\AIDACAT_ENG_v23\python\florence_caption.py" "M:\VS_Code_Projects\AIDACAT_ENG_v23\sample_images\1002674143_1b742ab4b8.jpg"
```

The first time, the model may need to download (requires an internet connection). That's normal. If everything works, an English image description appears in the terminal.

Semantic similarity:

```powershell
& "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe" "M:\VS_Code_Projects\AIDACAT_ENG_v23\python\semantic_similarity.py" "a dog running in a park" "a puppy playing outside"
```

Here too, the model may download the first time. If everything works, a number from 0 to 1 appears in the terminal.

If either of these tests fails, `single`, `web`, or `calibrate` mode will fail too.

---

## 6. Compiling the C++ program

The simplest way:

```powershell
.\build_windows.bat
```

If the build succeeds, this appears in the project folder:

```text
app.exe
```

The manual command:

```powershell
g++ -std=c++17 src\main.cpp src\TextProc.cpp src\Metrics.cpp src\FlickrLoader.cpp src\Florence.cpp src\Downloader.cpp src\WebParser.cpp src\Evaluation.cpp src\SemanticSimilarity.cpp src\Coefficients.cpp src\ImagePurpose.cpp src\ErrorAnalysis.cpp src\Calibration.cpp src\StatsUtil.cpp -I include -o app.exe
```

---

## 7. Running the modes from a terminal

### 7.1. Flickr8k validation mode

This mode tests the C++ metrics without Florence-2 or sentence-transformers - so it's fast (about 15-20 seconds for the full 8091-image dataset).

```powershell
.\app.exe experiment data\Flickr8k_text\Flickr8k.token.txt 0
```

Result:

```text
output\flickr_validation_results.csv
output\flickr_error_analysis.txt
```

This is the first mode you should test. If it works, the C++ metrics and the Flickr loader are working.

### 7.2. Single-image mode

This mode uses Florence-2 to generate a reference description, and sentence-transformers for semantic similarity.

```powershell
.\app.exe single "sample_images\1002674143_1b742ab4b8.jpg" "A child is playing outside" --python "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe"
```

Result:

```text
output\single_result.csv
```

Data flow:

```text
image
-> Florence-2 reference description
-> tested alt text
-> sentence-transformers semantic similarity
-> image purpose detection
-> C++ metrics with the correct weight profile
-> accessibility score
```

### 7.3. Web mode

This mode checks a simple web page.

```powershell
.\app.exe web "https://www.w3.org/WAI/tutorials/images/informative/" 5 --python "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe" --classify-complexity
```

Result:

```text
output\web_accessibility_results.csv
```

Downloaded images:

```text
output\downloaded_images
```

Important: the web parser is simple. It looks for normal `<img>` elements with `src` and `alt`. It does not execute JavaScript.

### 7.4. Coefficient calibration mode

This mode compares the program's score against human judgments. A detailed description is in section 9 of `USER_MANUAL.md` - **important to read about the time cost before running it**.

```powershell
.\app.exe calibrate data\Flickr8k_text\ExpertAnnotations.txt data\Flickr8k_text\Flickr8k.token.txt --limit 30 --python "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe"
```

Result:

```text
output\calibration_report.txt
```

---

## 8. Using Tasks in VS Code

The project includes:

```text
.vscode\tasks.json
```

This lets you run the most common commands from the VS Code menu.

Open:

```text
Terminal -> Run Task
```

Available tasks:

```text
01 Create Python venv
02 Install Python requirements
03 Check Python path
04 Test Florence only
05 Build C++
06 Run experiment mode (full dataset)
06b Run experiment mode (quick sample)
07 Run single mode
08 Run web mode
09 Run calibrate mode (quick, ~3 min)
10 Clean output
```

Recommended order for first-time setup:

```text
01 Create Python venv
02 Install Python requirements
03 Check Python path
04 Test Florence only
05 Build C++
06b Run experiment mode (quick sample)
07 Run single mode
08 Run web mode
09 Run calibrate mode (quick, ~3 min)
```

After the first setup, day to day you'll usually just need:

```text
05 Build C++
06 Run experiment mode (full dataset)
07 Run single mode
08 Run web mode
```

`tasks.json` uses one specific project path:

```text
M:\VS_Code_Projects\AIDACAT_ENG_v23
```

If the project is moved, this path needs to be updated in `.vscode/tasks.json` (everywhere it appears).

---

## 9. Possible problems and solutions

### 9.1. PowerShell blocks activating the environment

Error:

```text
Activate.ps1 cannot be loaded because running scripts is disabled on this system
```

Cause: PowerShell's security policy blocks `.ps1` scripts.

Fix for the current terminal only:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\test_env\Scripts\activate
```

Alternative without activating: use the full Python path, the same way `tasks.json` does.

---

### 9.2. `The system cannot find the path specified`

Cause: one of the paths is wrong or the file doesn't exist.

Check:

```powershell
Test-Path "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe"
Test-Path "M:\VS_Code_Projects\AIDACAT_ENG_v23\python\florence_caption.py"
Test-Path "M:\VS_Code_Projects\AIDACAT_ENG_v23\python\semantic_similarity.py"
Test-Path "M:\VS_Code_Projects\AIDACAT_ENG_v23\sample_images\1002674143_1b742ab4b8.jpg"
```

Every result should be:

```text
True
```

If one is `False`, fix that path or put the file in the right folder.

---

### 9.3. `ERROR: Florence did not return caption`

Cause: the Python script wasn't run or crashed with an error.

First test:

```powershell
& "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe" "M:\VS_Code_Projects\AIDACAT_ENG_v23\python\florence_caption.py" "M:\VS_Code_Projects\AIDACAT_ENG_v23\sample_images\1002674143_1b742ab4b8.jpg"
```

If this shows `ModuleNotFoundError`, install the libraries:

```powershell
& "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe" -m pip install -r requirements.txt
```

If it shows an error about `Florence2LanguageConfig` or similar, the `transformers` version is probably wrong - see section 9.5.

If a model download is happening, wait for it to finish.

---

### 9.4. `g++ is not recognized`

Cause: no C++ compiler is installed, or it's not on the Windows `PATH`.

Check:

```powershell
g++ --version
```

If the command isn't recognized, install MinGW/MSYS2 or another `g++` compiler and add its `bin` folder to the system `PATH`.

---

### 9.5. `ModuleNotFoundError` or `AttributeError: 'Florence2LanguageConfig' object has no attribute...`

First cause: the Python libraries aren't installed in the environment the program is using.

```powershell
& "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe" -m pip install -r requirements.txt
```

Second cause (more common): the `transformers` version is incompatible with Florence-2. This can happen if `sentence-transformers` was installed separately and pulled in a newer `transformers` (5.x) - Florence-2 only works with `transformers==4.49.0`. Fix - reinstall both libraries at the correct versions:

```powershell
& "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe" -m pip install "transformers==4.49.0" "sentence-transformers==4.1.0"
```

---

### 9.6. The Florence-2 or sentence-transformers download gets stuck

If a Hugging Face model download gets stuck, you can delete the incomplete files:

```powershell
Remove-Item "$env:USERPROFILE\.cache\huggingface\hub" -Recurse -Filter "*.incomplete"
```

Then run the relevant Python test again.

---

### 9.7. Web mode finds nothing

Possible reasons:

```text
the page has no normal <img> tags
images are loaded via JavaScript
the server is blocking the download
the image src is in an unusual format
```

This is an acceptable limitation. The program's goal isn't to be a full web crawler, but an alt-text evaluation prototype.

---

### 9.8. `calibrate` mode runs for a very long time

That's normal, not an error - every annotation row needs a separate Python process for semantic similarity. Use a smaller `--limit` value (see section 9 of `USER_MANUAL.md` for approximate timing).

---

## 10. Recommended demo sequence

For a demonstration, this sequence is recommended:

```text
1. Show the project structure.
2. Run 05 Build C++.
3. Run 06b Run experiment mode (quick sample) - for a quick check.
4. Run 06 Run experiment mode (full dataset) - show the full 8091-image analysis.
5. Open output/flickr_error_analysis.txt - show statistics, correlation, worst/best pairs.
6. Run 04 Test Florence only.
7. Run 07 Run single mode.
8. Open output/single_result.csv.
9. Run 08 Run web mode.
10. Open output/web_accessibility_results.csv - show the purpose and semantic columns.
11. Run 09 Run calibrate mode (quick, ~3 min).
12. Open output/calibration_report.txt - show the correlation before/after calibration.
```

This clearly demonstrates that the program has:

```text
a validation mode over the full dataset with error analysis
a practical single-image check with semantic similarity and purpose detection
a practical web page check
coefficient calibration against real human judgments
```
