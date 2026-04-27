# Makefile for ELZ v1.0
CONTIKI_PROJECT = elz-coordinator elz-lcn-node elz-nn-node
all: $(CONTIKI_PROJECT)

# Path to Contiki-NG root
CONTIKI = ../..

# Additional source files (linked into ALL targets)
PROJECT_SOURCEFILES += elz-csma-config.c
PROJECT_SOURCEFILES += elz-atpc.c
PROJECT_SOURCEFILES += elz-metrics.c

# Target platform
TARGET ?= cooja

include $(CONTIKI)/Makefile.include