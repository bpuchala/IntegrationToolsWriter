# make fw and bsw

ifeq ($(PTOOLS_PREFIX),)
	prefix = /usr/local
else
	prefix = $(PTOOLS_PREFIX)
endif

exec_prefix = $(prefix)

bindir = $(prefix)/bin

CXX ?= g++
CXXFLAGS += -O3
LDLIBS = -lboost_program_options -lboost_filesystem -lboost_system -lboost_regex -lcln -lginac

WRITER = $(CURDIR)/include/IntegrationToolsWriter
PSYM = $(WRITER)/psymfunction

INCLUDE = $(CURDIR)/include $(WRITER)

ifneq ($(PTOOLS_BOOST_PREFIX),)
	INCLUDE += $(PTOOLS_BOOST_PREFIX)/include
	LINK += $(PTOOLS_BOOST_PREFIX)/lib
endif

ifneq ($(PTOOLS_GINAC_PREFIX),)
	INCLUDE += $(PTOOLS_GINAC_PREFIX)/include
	LINK += $(PTOOLS_GINAC_PREFIX)/lib
endif

ifneq ($(PTOOLS_CLN_PREFIX),)
	INCLUDE += $(PTOOLS_CLN_PREFIX)/include
	LINK += $(PTOOLS_CLN_PREFIX)/lib
endif

CXXFLAGS += $(foreach i,$(INCLUDE),-I$(i))
LFLAGS += $(foreach i,$(LINK),-L$(i))

export

all: 
	cd src && $(MAKE)

install: 
	cd src && $(MAKE) install

uninstall:
	cd src && $(MAKE) uninstall

test: 
	cd tests && $(MAKE)

clean: 
	cd src && $(MAKE) clean
	cd tests && $(MAKE) clean
	rm -rf .cache .matplotlib


