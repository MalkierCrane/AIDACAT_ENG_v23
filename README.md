# AIDACAT_ENG_v23

This is a prototype for analyzing the accuracy and completeness of image descriptions for accessibility testing — compares a tested image's alternative text against a reference description and computes quality scores (lexical and semantic metrics, image purpose detection, accessibility score).

## Documentation

- [OVERVIEW.md](OVERVIEW.md) — project overview, modes, quick start, manual invocation
- [PROJECT_STRUCTURE.md](PROJECT_STRUCTURE.md) — project structure
- [USER_MANUAL.md](USER_MANUAL.md) — user manual
- [VS_CODE_GUIDE.md](VS_CODE_GUIDE.md) — VS Code guide

## Quick start

```powershell
.\build_windows.bat
.\app.exe experiment data\Flickr8k_text\Flickr8k.token.txt 0
```

See [OVERVIEW.md](OVERVIEW.md) for details.

## Dataset

The Flickr8k dataset (images and reference descriptions) used in the `data/` folder was NOT created by, nor is it owned by this project's author. It was downloaded from the GitHub repository [goodwillyoga/Flickr8k_dataset] - URL is (https://github.com/goodwillyoga/Flickr8k_dataset), which is based on the M. Hodosh, P. Young, and J. Hockenmaier paper "Framing Image Description as a Ranking Task: Data, Models and Evaluation Metrics" - the paper can be found here: https://www.jair.org/index.php/jair/article/view/10833. Parts not needed for this project were removed from the downloaded copy. All rights to the original dataset belong to its authors/maintainers — see the source repository for more detailed information about the dataset.
