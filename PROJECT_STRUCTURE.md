# Project Structure

This file describes the project's folders, files, and their purpose.

Expected project path:

```text
M:\VS_Code_Projects\AIDACAT_ENG_v23
```

---

## 1. Overall structure

```text
AIDACAT_ENG_v23/
├── .vscode/
│   └── tasks.json
├── include/
│   ├── Types.h
│   ├── TextProc.h
│   ├── Metrics.h
│   ├── FlickrLoader.h
│   ├── Florence.h
│   ├── Downloader.h
│   ├── WebParser.h
│   ├── SemanticSimilarity.h
│   ├── Coefficients.h
│   ├── ImagePurpose.h
│   ├── ErrorAnalysis.h
│   ├── Calibration.h
│   ├── StatsUtil.h
│   └── Evaluation.h
├── src/
│   ├── main.cpp
│   ├── TextProc.cpp
│   ├── Metrics.cpp
│   ├── FlickrLoader.cpp
│   ├── Florence.cpp
│   ├── Downloader.cpp
│   ├── WebParser.cpp
│   ├── SemanticSimilarity.cpp
│   ├── Coefficients.cpp
│   ├── ImagePurpose.cpp
│   ├── ErrorAnalysis.cpp
│   ├── Calibration.cpp
│   ├── StatsUtil.cpp
│   └── Evaluation.cpp
├── python/
│   ├── florence_caption.py
│   └── semantic_similarity.py
├── data/
│   ├── Flickr8k_sample.token.txt
│   ├── Flickr8k_text/
│   │   ├── Flickr8k.token.txt
│   │   ├── Flickr8k.lemma.token.txt
│   │   ├── ExpertAnnotations.txt
│   │   ├── CrowdFlowerAnnotations.txt
│   │   ├── Flickr_8k.trainImages.txt
│   │   ├── Flickr_8k.devImages.txt
│   │   ├── Flickr_8k.testImages.txt
│   │   └── readme.txt
│   └── Flickr8k_images/
│       └── Flicker8k_Dataset/   (8091 images)
├── sample_images/
│   ├── 1002674143_1b742ab4b8.jpg
│   ├── 1003163366_44323f5815.jpg
│   └── 1015118661_980735411b.jpg
├── output/
│   └── downloaded_images/
├── build_windows.bat
├── requirements.txt
├── README.md
├── OVERVIEW.md
├── USER_MANUAL.md
├── PROJECT_STRUCTURE.md
└── VS_CODE_GUIDE.md
```

Note: this copy of the project does NOT contain `test_env/` (the Python environment) or `app.exe` - you create both yourself by following the `VS_CODE_GUIDE.md` instructions. This is deliberate, because a Python environment isn't safely relocatable to another folder.

---

## 2. `.vscode` folder

### `.vscode/tasks.json`

This file holds VS Code tasks, so you don't have to type long commands into the terminal by hand.

Tasks:

| Task | Purpose | Required |
|---|---|---|
| `01 Create Python venv` | Creates the Python virtual environment | Needed for Florence/semantic modes |
| `02 Install Python requirements` | Installs PyTorch, Transformers, sentence-transformers and other libraries | Needed for Florence/semantic modes |
| `03 Check Python path` | Checks the most important paths | Useful for diagnostics |
| `04 Test Florence only` | Runs Florence without C++ | Useful before `single`, `web`, `calibrate` |
| `05 Build C++` | Compiles the C++ program | Vital |
| `06 Run experiment mode (full dataset)` | Runs Flickr8k validation on the whole dataset | Vital for validation |
| `06b Run experiment mode (quick sample)` | Runs Flickr8k validation on a 5-image sample | Useful for a quick check |
| `07 Run single mode` | Runs the single-image check | Important for a practical demo |
| `08 Run web mode` | Runs the web page check | Important for a practical demo |
| `09 Run calibrate mode` | Runs coefficient calibration | Important to justify the method |
| `10 Clean output` | Deletes previous CSV/TXT results | Not required |

If the project is moved to another folder, update the path in this file:

```text
M:\VS_Code_Projects\AIDACAT_ENG_v23
```

---

## 3. `include` folder

This folder holds the C++ class and struct declarations.

### `Types.h`

Holds the shared data structures.

```text
ImageItem      - one image found on a web page
FlickrItem     - one Flickr8k image with its descriptions
MetricResult   - the result of a metric computation
WeightProfile  - the weight profile for one image purpose
CorpusStats    - corpus-wide n-gram statistics for CIDEr TF-IDF
```

A vital file. Do not delete it.

### `TextProc.h`

Declares the text-processing class (lowercasing, punctuation stripping, stop words, simple normalization). A vital file, unchanged from the earlier version.

### `Metrics.h`

Declares the metric computations.

```text
Precision, Recall, F1
BLEU-like
METEOR (real word alignment + fragmentation penalty)
ROUGE-L
CIDER (with and without corpus TF-IDF)
attachSemantic - adds the semantic metric and computes the final score using a weight profile
```

A vital file, the program's research core.

### `FlickrLoader.h`

Declares Flickr8k text file loading. Needed for `experiment` and `calibrate` mode.

### `Florence.h`

Declares the C++ class that runs the Python Florence script (both for generating descriptions AND for counting regions for complexity detection). Needed for `single` and `web` modes.

### `Downloader.h`

Declares simple file download functionality (curl). Needed for `web` mode.

### `WebParser.h`

Declares a simple HTML parser that finds `<img src="..." alt="...">` and checks whether an image is wrapped in an `<a>` link. Needed for `web` mode.

### `SemanticSimilarity.h`

**New file.** Declares the C++ class that runs the Python sentence-transformers script for computing semantic similarity. Built on the same pattern as `Florence.h`. Needed for `single`, `web`, and `calibrate` modes.

### `Coefficients.h`

**New file.** Declares the class that holds the 4 named weight profiles (decorative/informative/functional/complex).

### `ImagePurpose.h`

**New file.** Declares the class that detects an image's purpose from its alt text, link context, and (optionally) Florence's region count.

### `ErrorAnalysis.h`

**New file.** Declares the class that runs a detailed error analysis over `experiment` mode's results (statistics, level distribution, correlation, worst/best pairs).

### `StatsUtil.h`

**New file.** Declares simple statistics helper functions (mean, median, standard deviation, Pearson correlation), shared by `ErrorAnalysis` and `Calibration`.

### `Calibration.h`

**New file.** Declares the class that compares the program's score against ExpertAnnotations.txt/CrowdFlowerAnnotations.txt human judgments and searches for an improved weight profile.

### `Evaluation.h`

Declares the program's main logic class. It ties all the other classes together and implements all 4 modes. A vital file.

---

## 4. `src` folder

This folder holds the C++ implementations - each `include/*.h` file has a matching `.cpp` file with the same role. See section 3 for details.

Notes on specific files:

```text
main.cpp        - the program's entry point, command-line argument handling, the 4 modes
TextProc.cpp    - unchanged from the earlier versions
FlickrLoader.cpp - unchanged from the earlier versions
Downloader.cpp  - unchanged from the earlier versions
Florence.cpp    - fixed the diagnostic-line pollution bug (see OVERVIEW.md)
Metrics.cpp     - the longest file, holds all the metric math
Evaluation.cpp  - the second-longest file, ties everything together
```

---

## 5. `python` folder

### `python/florence_caption.py`

A Python script that loads the Florence-2 model and generates an image description OR counts regions (depending on `--task caption` / `--task regions`). 
The C++ program runs it through `Florence.cpp`. Needed for `single`, `web`, and `calibrate` modes.

### `python/semantic_similarity.py`

**New file.** A Python script that loads a sentence-transformers model and computes semantic similarity between two texts. The C++ program runs it through `SemanticSimilarity.cpp`.

If you only use `experiment` mode, you don't need either of these files.

---

## 6. `data` folder

### `data/Flickr8k_sample.token.txt`

A small Flickr8k text file sample (5 images). Needed for quickly checking `experiment` mode.

### `data/Flickr8k_text/Flickr8k.token.txt`

The **full** Flickr8k text file (8091 images, 5 descriptions each). This is the new default source for `experiment` mode.

### `data/Flickr8k_text/ExpertAnnotations.txt` and `CrowdFlowerAnnotations.txt`

Real human judgments from the original Flickr8k dataset paper. Needed for `calibrate` mode (see section 9 of `USER_MANUAL.md`).

### `data/Flickr8k_images/Flicker8k_Dataset/`

8091 image files. Only needed for `calibrate` mode (so the images being analyzed can be viewed manually if desired) - `experiment` mode only uses the text, not the images themselves.

---

## 7. `sample_images` folder

Holds test images for `single` mode and Florence testing. These files aren't part of the program's core, but they're useful for a demo.

---

## 8. `output` folder

The program saves its results here.

Possible files:

```text
flickr_validation_results.csv
flickr_error_analysis.txt
single_result.csv
web_accessibility_results.csv
calibration_report.txt
calibration_report_crowdflower.txt
```

`output/downloaded_images` holds the images downloaded in web mode.

---

## 9. Root files

### `build_windows.bat`

Compiles the C++ program on Windows. Vital for practical use.

### `requirements.txt`

Holds the list of Python libraries for the Florence-2 and sentence-transformers modules, with exactly matched versions. Needed for `single`, `web`, and `calibrate` modes.

### `README.md`

The GitHub landing page - a short project blurb, dataset attribution, and links to the other docs.

### `OVERVIEW.md`

A detailed project overview: what the program does, modes, quick start, manual invocation.

### `USER_MANUAL.md`

A detailed guide for running the program.

### `PROJECT_STRUCTURE.md`

This file. Describes the structure and the purpose of each file.

### `VS_CODE_GUIDE.md`

Special instructions for setting up and testing the program in Visual Studio Code, including the required VS Code extensions.

---

## 10. What's safe to remove if needed

Can be removed:

```text
extra images in sample_images
the contents of the output folder
the 10 Clean output task
VS_CODE_GUIDE.md, if documentation isn't needed in the project
data/Flickr8k_images/Flicker8k_Dataset/, if you don't use calibrate mode (but it's
    still useful if you want to view the images calibrate is analyzing)
```

Should NOT be removed:

```text
src
include
python/florence_caption.py
python/semantic_similarity.py, if you use single/web/calibrate mode
requirements.txt
build_windows.bat
data/Flickr8k_text/Flickr8k.token.txt
data/Flickr8k_text/ExpertAnnotations.txt and CrowdFlowerAnnotations.txt, if you use calibrate mode
.vscode/tasks.json, if you want convenient VS Code testing
```

If you remove `python/florence_caption.py`, only `experiment` mode still works.

If you remove `FlickrLoader`, the validation and calibration modes disappear.
