MODULES = pg_graph_component
EXTENSION = pg_graph_component
EXTENSION_VERSION = 1.0.0
DATA = $(EXTENSION)--$(EXTENSION_VERSION).sql

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

release:
	git archive --format zip --prefix=$(EXTENSION)-$(EXTENSION_VERSION)/ --output $(EXTENSION)-$(EXTENSION_VERSION).zip master

.PHONY: release

CPLUSPLUSFLAGS = -W -Wall -O3 -msse4.2 -march=native -DNDEBUG
CPLUSPLUSFLAGS += $(PG_CPPFLAGS)

OCC := $(CC)
CC = $(CXX)


%.o : %.c
	$(OCC) $(CPPFLAGS) -fPIC -O3 -msse4.2 -march=native -c -o $@ $<

%.o : %.cpp
	$(CXX) $(CPLUSPLUSFLAGS) $(CPPFLAGS) -fpic -c -o $@ $<