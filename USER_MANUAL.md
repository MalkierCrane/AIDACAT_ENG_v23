# User Manual

This manual describes how to run the program and use its modes. The program is meant for Analysis of Image Descriptions Accuracy and Completeness for Accessibility Testing.

Expected project path:

```text
M:\VS_Code_Projects\AIDACAT_ENG_v23
```

---

## 1. Purpose of the program

The program's main job is to compare a tested image's alternative text against a reference description and compute description quality scores.

The program does not train any AI model itself. Florence-2 is used only as an external Python module for generating the reference description in the practical modes. Sentence-transformers is used as a second external Python module for computing semantic similarity.

---

## 2. Program modes

There are four main modes.

| Mode | Command | Purpose |
|---|---|---|
| Flickr8k validation | `experiment` | Checks metrics against human-written Flickr8k descriptions, with error analysis |
| Single-image check | `single` | Checks one image against one user-supplied alt text |
| Web page check | `web` | Extracts `<img>` and `alt` text from a web page |
| Coefficient calibration | `calibrate` | Compares the program's score against human judgments |

---

## 3. Required parts

To run only `experiment` mode, you need:

```text
a C++ compiler
data/Flickr8k_text/Flickr8k.token.txt (or the small data/Flickr8k_sample.token.txt for a quick sample)
app.exe
```

To run `single`, `web` and `calibrate` mode, you additionally need:

```text
a Python virtual environment
PyTorch
Transformers (for Florence-2 reference descriptions)
sentence-transformers (for semantic similarity)
```

---

## 4. Setting up the Python environment

From the project folder:

```powershell
cd "M:\VS_Code_Projects\AIDACAT_ENG_v23"
python -m venv test_env
```

Installing libraries without activating the environment:

```powershell
& "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe" -m pip install --upgrade pip
& "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe" -m pip install -r requirements.txt
```

**Important note about versions:** `requirements.txt` deliberately pins both `transformers==4.49.0` and `sentence-transformers==4.1.0` to exact versions. If you install `sentence-transformers` separately (outside of `requirements.txt`) or upgrade it later, `pip` may pull in a newer `transformers` (5.x), which **breaks Florence-2** (Florence-2 relies on `trust_remote_code=True` custom code that is not compatible with `transformers` 5.x). If that happens, reinstall the exact pinned versions:

```powershell
& "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe" -m pip install "transformers==4.49.0" "sentence-transformers==4.1.0"
```

Testing Florence:

```powershell
& "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe" "M:\VS_Code_Projects\AIDACAT_ENG_v23\python\florence_caption.py" "M:\VS_Code_Projects\AIDACAT_ENG_v23\sample_images\1002674143_1b742ab4b8.jpg"
```

---

## 5. Compiling the C++ program

From the project folder:

```powershell
.\build_windows.bat
```

On a successful build, this creates:

```text
app.exe
```

---

## 6. Flickr8k validation mode

Command (full dataset, 8091 images):

```powershell
.\app.exe experiment data\Flickr8k_text\Flickr8k.token.txt 0
```

For a quick check with a small sample (5 images):

```powershell
.\app.exe experiment data\Flickr8k_sample.token.txt 5
```

The third parameter is the limit - how many images to load (`0` = all). The extra flag `--top-n N` (default 10) controls how many worst/best pairs are shown in the error analysis.

What this mode does:

```text
loads Flickr8k descriptions
groups them by image name
uses one description as the reference
uses the other descriptions as test texts
computes metrics (WITHOUT semantic similarity - see note below)
saves CSV results
runs a detailed error analysis: statistics, level distribution, correlation, worst/best pairs
```

**Why no semantic similarity here:** the full dataset has about 32,000 comparable pairs. Computing semantic similarity spawns a separate Python process per pair - doing that 32,000 times would take several hours. So `experiment` mode (just like in the earlier versions) never calls Python at all - it stays a pure 
C++ text-metric comparison, which lets it run on the full dataset in a few seconds (about 15-20 seconds on a mid-range machine).

Results:

```text
output\flickr_validation_results.csv    - metrics for every pair
output\flickr_error_analysis.txt        - error analysis report
```

---

## 7. Single-image mode

Command:

```powershell
.\app.exe single "sample_images\1002674143_1b742ab4b8.jpg" "A child is playing outside" --python "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe"
```

The extra flag `--classify-complexity` turns on Florence region counting (for complexity detection) - it requires one more Florence call, so it's off by default.

What this mode does:

```text
takes a local image
runs the Florence-2 Python script, gets a reference description
runs the sentence-transformers Python script, computes semantic similarity
detects the image's purpose (decorative/informative/functional/complex) from the alt text
picks the weight profile matching that purpose
computes accessibility scores
saves the CSV result
```

Result:

```text
output\single_result.csv
```

---

## 8. Web mode

Command:

```powershell
.\app.exe web "https://www.w3.org/WAI/tutorials/images/informative/" 5 --python "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe" --classify-complexity
```

What this mode does:

```text
downloads the HTML
finds <img> tags, including checking whether they're wrapped in an <a> link
extracts src and alt attributes
downloads the images
Florence-2 generates reference descriptions
sentence-transformers computes semantic similarity
detects each image's purpose and picks a matching weight profile
C++ computes the metrics
saves the page's accessibility results
```

Result:

```text
output\web_accessibility_results.csv
```

Downloaded images:

```text
output\downloaded_images
```

---

## 9. Coefficient calibration mode

This mode compares the program's computed score against real human judgments from the original Flickr8k dataset paper (Hodosh, Young and Hockenmaier, 2013).
Original paper may be found here: https://www.jair.org/index.php/jair/article/view/10833

Command:

```powershell
.\app.exe calibrate data\Flickr8k_text\ExpertAnnotations.txt data\Flickr8k_text\Flickr8k.token.txt --limit 30 --python "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe"
```

Extra flags:

```text
--limit N              how many annotation rows to process (default 100 - without a limit
                        the full dataset (5822 rows) would take several hours, see below)
--crowdflower           also calibrate against CrowdFlowerAnnotations.txt
--crowdflower-file <path>  a different CrowdFlower file path
```

**How long this takes:** every annotation row needs one semantic-similarity Python call (reloading the model each time takes about 5 seconds). In testing, 100 rows took about 9 minutes. So:

```text
--limit 30    about 2-3 minutes   - for a quick check
--limit 100   about 8-9 minutes   - the default, a reasonable compromise
--limit 500   about 45 minutes    - for a more serious analysis
--limit 0     (all 5822 rows)     - several hours, only run this with time to spare
```

What this mode does:

```text
loads all Flickr8k images and descriptions (no limit)
loads ExpertAnnotations.txt (and, if requested, CrowdFlowerAnnotations.txt)
for every annotation row: finds the judged image's reference description and the
    candidate description being judged, computes our metrics, compares to the human score
computes the correlation between our final_score and the human score (before calibration)
searches for an improved weight profile using coordinate ascent (adjusting each weight
    one step at a time)
computes the correlation with the new profile (after calibration)
writes both results and the suggested profile to a report
```

**Important note about ExpertAnnotations.txt's structure:** every row judges how well ONE image's description (column 2) describes ANOTHER image (column 1) - in most cases (about 97%) these are DIFFERENT images (this is the original paper's "ranking task" structure - it tests whether a system correctly tells a right description apart from a wrong one). The program uses the judged image's first human description as the reference text, and compares it against the named candidate description - the same approach `experiment` mode already uses.

Result:

```text
output\calibration_report.txt                  - ExpertAnnotations.txt results
output\calibration_report_crowdflower.txt       - CrowdFlowerAnnotations.txt results (if --crowdflower)
```

**Using the results:** the report offers a new weight profile as a ready-to-copy list of numbers. The program does NOT apply these numbers automatically - to use them, copy them by hand into the `informative` profile in `src/Coefficients.cpp` and rebuild. This is deliberately not automatic, so the person decides for themselves whether the improved correlation justifies the change.

### Where to find more comparison data

1. **`ExpertAnnotations.txt` / `CrowdFlowerAnnotations.txt`** - already included in this project (`data/Flickr8k_text/`), nothing to download. These are real human judgments from Hodosh, Young and Hockenmaier (2013), *"Framing Image Description as a Ranking Task"*, JAIR, vol. 47.
2. **The W3C WAI images tutorial** - `https://www.w3.org/WAI/tutorials/images/` - the same site already used as this program's `web` mode demo URL. It has separate subpages for exactly the decorative/informative/functional/complex categories (plus text/group) - a free, standards-body-curated reference to check the `ImagePurpose` classifier's output against.
3. **Further reading only (not integrated into the code):** MS COCO Captions (a larger, human-evaluated caption dataset), Flickr30k (Flickr8k's bigger sibling), PASCAL-50S/ABSTRACT-50S (the datasets the original CIDEr paper's authors used specifically to measure metric-vs-human correlation - the same thing this project's `calibrate` mode does).

---

## 10. Meaning of the metrics

CSV files and console output show the following values:

| Value | Meaning |
|---|---|
| Precision | How much of the tested text matches the reference text |
| Recall | How much of the reference text is covered by the tested text |
| F1 | A balanced combination of Precision and Recall |
| BLEU | An n-gram (1-4 word sequence) similarity score with a shortness penalty |
| METEOR | A word-alignment score (exact + synonym matching) with a fragmentation penalty |
| ROUGE-L | Longest common subsequence similarity |
| CIDEr | N-gram content similarity with corpus-wide TF-IDF weighting (rarer n-grams get more weight) |
| Semantic | Semantic similarity between the two texts' meaning (sentence-transformers), 0 to 1 - **always 0 in `experiment` mode** (see section 6) |
| Final | Overall accessibility score from 0 to 100, computed using the weight profile matching the image's purpose |
| Level | Text category: `insufficient` / `weak` / `average` / `good` / `excellent` |
| Purpose | Image purpose: `decorative` / `informative` / `functional` / `complex` / `n/a` (when purpose isn't detected, e.g. in `experiment` mode) |

### Weight profiles

Each `purpose` category has its own weight profile (see `src/Coefficients.cpp`):

| Profile | Reasoning |
|---|---|
| `informative` | A balanced profile, emphasizing precision and semantic similarity. **The only one empirically calibrated.** |
| `decorative` | Emphasizes semantic similarity and precision - for a decorative image, correct alt text is either empty or short and precise, not a long description. |
| `functional` | The highest semantic-similarity weight - what matters is that alt text correctly describes the ACTION (a button's or link's purpose), not the visual appearance. |
| `complex` | A higher recall weight - what matters is that the description covers ALL the content (e.g. a chart's data), not just part of it. |

---

## 11. VS Code Tasks

The project includes:

```text
.vscode\tasks.json
```

This lets you run commands from VS Code:

```text
Terminal -> Run Task
```

The most important tasks:

```text
05 Build C++
06 Run experiment mode (full dataset)
06b Run experiment mode (quick sample)
07 Run single mode
08 Run web mode
09 Run calibrate mode (quick, ~3 min)
```

For first-time setup:

```text
01 Create Python venv
02 Install Python requirements
03 Check Python path
04 Test Florence only
```

---

## 12. Limitations

This program is a prototype, not a full industrial accessibility audit tool.

Main limitations:

```text
the web parser is simple, there is no real DOM tree
JavaScript is not executed
CSS background images are not analyzed
the Florence-2 reference is not a human-verified absolute ground truth
image purpose detection is a simplified heuristic (length and keyword checks), not a full
    implementation of the W3C WAI decision tree
only the "informative" weight profile is empirically calibrated
calibrate mode is slow at a large --limit (see section 9)
```
