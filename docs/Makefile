HTML_DOC_DIR = api
MANUAL_DIR = manual
PREFIX = ..

#
# Should not need to edit below this line
#

docs: html_manual api_docs pdf_manual latex_clean

pdf_manual:
	cd tex-files; latex manual; latex manual; latex manual; dvipdf manual.dvi; mv manual.pdf .. 

html_manual:
	cd tex-files; latex2html -split 3 -no_math -html_version 3.2 -dir ../${MANUAL_DIR} -mkdir -contents_in_navigation manual

api_docs:
	doxygen doxygen.config

latex_clean:
	rm -f tex-files/*.aux
	rm -f tex-files/*.out
	rm -f tex-files/*.dvi
	rm -f tex-files/*.log
	rm -f tex-files/*.toc

clean:
	rm -rf ${HTML_DOC_DIR}
	rm -rf ${MANUAL_DIR}
	rm -f manual.pdf

