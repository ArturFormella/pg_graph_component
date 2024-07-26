MODULES = graph_component

EXTENSION    = $(shell grep -m 1 '"name":' META.json | \
               sed -e 's/[[:space:]]*"name":[[:space:]]*"\([^"]*\)",/\1/')
EXTVERSION   = $(shell grep -m 1 '[[:space:]]\{6,8\}"version":' META.json | \
               sed -e 's/[[:space:]]*"version":[[:space:]]*"\([^"]*\)",\{0,1\}/\1/')
DISTVERSION  = $(shell grep -m 1 '[[:space:]]\{3\}"version":' META.json | \
               sed -e 's/[[:space:]]*"version":[[:space:]]*"\([^"]*\)",\{0,1\}/\1/')

DATA 		    = $(wildcard *--*.sql)
PG_CONFIG   ?= pg_config
PG91         = $(shell $(PG_CONFIG) --version | grep -qE " 8\.| 9\.0" && echo no || echo yes)

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

CPLUSPLUSFLAGS = -W -Wall -O3 -msse4.2 -march=native -DNDEBUG
CPLUSPLUSFLAGS += $(PG_CPPFLAGS)

OCC := $(CC)
CC = $(CXX)

all: $(EXTENSION)--$(EXTVERSION).sql

$(EXTENSION)--$(EXTVERSION).sql: $(EXTENSION).sql
	cp $< $@

%.o : %.c
	$(OCC) $(CPPFLAGS) -fPIC -O3 -msse4.2 -march=native -c -o $@ $<

%.o : %.cpp
	$(CXX) $(CPLUSPLUSFLAGS) $(CPPFLAGS) -fpic -c -o $@ $<
  
dist:
	git archive --format zip --prefix=$(EXTENSION)-$(DISTVERSION)/ -o $(EXTENSION)-$(DISTVERSION).zip HEAD
