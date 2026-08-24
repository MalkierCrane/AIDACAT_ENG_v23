# Analysis of Image Descriptions Accuracy and Completeness for Accessibility Testing Program

This project is the practical project on the Analysis of Image Descriptions Accuracy and Completeness for Accessibility Testing.

Expected project path in this version:

```text
M:\VS_Code_Projects\AIDACAT_ENG_v23
```

---

## What the program does

The program compares a tested image's alternative text against a reference description and computes quality scores.

Main idea:

```text
reference description
+
tested alt text
-> C++ metrics (lexical + semantic similarity)
-> image purpose detection (decorative / informative / functional / complex)
-> a purpose-matched weight profile
-> accessibility score
```

Florence-2 is used only in the practical modes, as an external Python module for generating the reference description. Sentence-transformers is used as a second external Python module for computing semantic similarity.

---

## What's new in this version

Compared to the earlier versions:

```text
experiment mode now defaults to the FULL Flickr8k dataset (8091 images) instead of a
    5-image sample, and runs a detailed error analysis after every run
fixed a bug where Florence-2's diagnostic line ("Florence device: cpu") leaked into the
    captured description and polluted every metric
added a semantic similarity metric (sentence-transformers) alongside the existing
    lexical metrics
METEOR now uses real word alignment and a fragmentation penalty, not just a formula
CIDER now uses corpus-wide TF-IDF weights (when the full Flickr8k corpus is available)
added image purpose detection (decorative / informative / functional / complex),
    modeled loosely on the W3C WAI alt-text decision tree
each purpose has its own weight profile, and the "informative" profile is calibrated
    against ExpertAnnotations.txt human judgments (new "calibrate" mode)
score categories are now in English (insufficient/weak/average/good/excellent) instead of Latvian
```

---

## Main modes

| Mode | Command | Purpose |
|---|---|---|
| `experiment` | Flickr8k validation | Checks metrics against human-written descriptions, full dataset, with error analysis |
| `single` | Single-image check | Florence-2 generates a reference description for one image |
| `web` | Web page check | Analyzes `<img>` and `alt` text from a URL |
| `calibrate` | Coefficient calibration | Compares our score against ExpertAnnotations.txt / CrowdFlowerAnnotations.txt human judgments |

---

## Quick start in VS Code

1. Open the project in VS Code:

```text
M:\VS_Code_Projects\AIDACAT_ENG_v23
```

2. Use the VS Code menu:

```text
Terminal -> Run Task
```

3. First time, run in order:

```text
01 Create Python venv
02 Install Python requirements
03 Check Python path
04 Test Florence only
05 Build C++
```

4. Test the modes:

```text
06 Run experiment mode (full dataset)
07 Run single mode
08 Run web mode
09 Run calibrate mode (quick, ~3 min)
```

---

## Manual compilation

```powershell
.\build_windows.bat
```

---

## Manual mode invocation

Flickr8k validation (full dataset, with error analysis):

```powershell
.\app.exe experiment data\Flickr8k_text\Flickr8k.token.txt 0
```

Single-image check:

```powershell
.\app.exe single "sample_images\1002674143_1b742ab4b8.jpg" "A child is playing outside" --python "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe"
```

Web check:

```powershell
.\app.exe web "https://www.w3.org/WAI/tutorials/images/informative/" 5 --python "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe" --classify-complexity
```

Coefficient calibration:

```powershell
.\app.exe calibrate data\Flickr8k_text\ExpertAnnotations.txt data\Flickr8k_text\Flickr8k.token.txt --limit 30 --python "M:\VS_Code_Projects\AIDACAT_ENG_v23\test_env\Scripts\python.exe"
```

---

## Documentation

Further reading:

```text
PROJECT_STRUCTURE.md
USER_MANUAL.md
VS_CODE_GUIDE.md
```

---

## Limitations

This program is prototype, not a full industrial web accessibility crawler.

Main limitations:

```text
the HTML parser is simplified, there is no real DOM tree
JavaScript is not executed
CSS background-image is not analyzed
the Florence-2 reference is not a human-verified absolute ground truth
image purpose detection is a simplified heuristic, not a full implementation of the WAI
    decision tree
only the "informative" weight profile is empirically calibrated - the other 3 are
    reasoned assumptions
calibrate mode is slow at a large --limit, because every row needs a separate Python
    process for the semantic similarity computation (see USER_MANUAL.md)
```
