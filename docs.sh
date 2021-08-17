#!/bin/sh
set -e

mkdir -p docs/pdf
#Write the latex in docs/latex as a pdf in docs/pdf
pdflatex -output-directory docs/pdf docs/latex/main.tex

mkdir -p docs/html
#Write the latex in docs/latex as html files in docs/html
latex2html  -dir docs/html -html_version 5.0,unicode docs/latex/main.tex