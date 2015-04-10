
SHELL = /bin/sh

#### Start of system configuration section. ####

srcdir = trema
topdir = /home/yasuhito/.rvm/rubies/ruby-1.8.7-p371/lib/ruby/1.8/x86_64-linux
hdrdir = $(topdir)
VPATH = $(srcdir):$(topdir):$(hdrdir)
exec_prefix = $(prefix)
prefix = $(DESTDIR)/home/yasuhito/.rvm/rubies/ruby-1.8.7-p371
vendorlibdir = $(vendordir)/$(ruby_version)
docdir = $(datarootdir)/doc/$(PACKAGE)
sitedir = $(libdir)/ruby/site_ruby
oldincludedir = $(DESTDIR)/usr/include
mandir = $(datarootdir)/man
includedir = $(prefix)/include
vendorarchdir = $(vendorlibdir)/$(sitearch)
localstatedir = $(prefix)/var
sharedstatedir = $(prefix)/com
rubylibdir = $(libdir)/ruby/$(ruby_version)
sitelibdir = $(sitedir)/$(ruby_version)
infodir = $(datarootdir)/info
localedir = $(datarootdir)/locale
datadir = $(datarootdir)
sbindir = $(exec_prefix)/sbin
vendordir = $(libdir)/ruby/vendor_ruby
libexecdir = $(exec_prefix)/libexec
bindir = $(exec_prefix)/bin
pdfdir = $(docdir)
libdir = $(exec_prefix)/lib
dvidir = $(docdir)
sitearchdir = $(sitelibdir)/$(sitearch)
htmldir = $(docdir)
archdir = $(rubylibdir)/$(arch)
sysconfdir = $(prefix)/etc
psdir = $(docdir)
datarootdir = $(prefix)/share

CC = gcc
LIBRUBY = $(LIBRUBY_SO)
LIBRUBY_A = lib$(RUBY_SO_NAME)-static.a
LIBRUBYARG_SHARED = -Wl,-R -Wl,$(libdir) -L$(libdir) -l$(RUBY_SO_NAME)
LIBRUBYARG_STATIC = -l$(RUBY_SO_NAME)-static

RUBY_EXTCONF_H = 
CFLAGS   =  -fPIC -g -std=gnu99 -D_GNU_SOURCE -fno-strict-aliasing -Wall -Wextra -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wfloat-equal -Wpointer-arith -Werror 
INCFLAGS = -I. -I. -I/home/yasuhito/.rvm/rubies/ruby-1.8.7-p371/lib/ruby/1.8/x86_64-linux -I.
DEFS     = 
CPPFLAGS =  -I/home/yasuhito/play/trema/objects/openflow -I/home/yasuhito/play/trema/src/lib   
CXXFLAGS = $(CFLAGS) 
ldflags  = -Wl,-Bsymbolic
dldflags = 
archflag = 
DLDFLAGS = $(ldflags) $(dldflags) $(archflag)
LDSHARED = $(CC) -shared
AR = ar
EXEEXT = 

RUBY_INSTALL_NAME = ruby
RUBY_SO_NAME = ruby
arch = x86_64-linux
sitearch = x86_64-linux
ruby_version = 1.8
ruby = /home/yasuhito/.rvm/rubies/ruby-1.8.7-p371/bin/ruby
RUBY = $(ruby)
RM = rm -f
MAKEDIRS = mkdir -p
INSTALL = /usr/bin/install -c
INSTALL_PROG = $(INSTALL) -m 0755
INSTALL_DATA = $(INSTALL) -m 644
COPY = cp

#### End of system configuration section. ####

preload = 

libpath = . $(libdir) /home/yasuhito/play/trema/objects/lib
LIBPATH =  -L. -L$(libdir) -Wl,-R$(libdir) -L/home/yasuhito/play/trema/objects/lib -Wl,-R/home/yasuhito/play/trema/objects/lib
DEFFILE = 

CLEANFILES = mkmf.log
DISTCLEANFILES = 

extout = 
extout_prefix = 
target_prefix = 
LOCAL_LIBS = 
LIBS = $(LIBRUBYARG_SHARED) -ltrema -lsqlite3 -ldl -lrt -lpthread  -lrt -ldl -lcrypt -lm   -lc
SRCS = get-config-request.c echo.c barrier-request.c action-common.c switch-event.c queue-get-config-request.c switch.c trema.c queue-get-config-reply.c echo-request.c default-logger.c flow-removed.c stats-request.c openflow-error.c switch-disconnected.c port-status.c stats-reply.c set-config.c barrier-reply.c hello.c match.c list-switches-reply.c error.c echo-reply.c port-mod.c features-request.c flow-mod.c controller.c trema-ruby-utils.c get-config-reply.c port.c packet-in.c features-reply.c vendor.c
OBJS = get-config-request.o echo.o barrier-request.o action-common.o switch-event.o queue-get-config-request.o switch.o trema.o queue-get-config-reply.o echo-request.o default-logger.o flow-removed.o stats-request.o openflow-error.o switch-disconnected.o port-status.o stats-reply.o set-config.o barrier-reply.o hello.o match.o list-switches-reply.o error.o echo-reply.o port-mod.o features-request.o flow-mod.o controller.o trema-ruby-utils.o get-config-reply.o port.o packet-in.o features-reply.o vendor.o
TARGET = trema
DLLIB = $(TARGET).so
EXTSTATIC = 
STATIC_LIB = 

BINDIR        = $(bindir)
RUBYCOMMONDIR = $(sitedir)$(target_prefix)
RUBYLIBDIR    = $(sitelibdir)$(target_prefix)
RUBYARCHDIR   = $(sitearchdir)$(target_prefix)

TARGET_SO     = $(DLLIB)
CLEANLIBS     = $(TARGET).so $(TARGET).il? $(TARGET).tds $(TARGET).map
CLEANOBJS     = *.o *.a *.s[ol] *.pdb *.exp *.bak

all:		$(DLLIB)
static:		$(STATIC_LIB)

clean:
		@-$(RM) $(CLEANLIBS) $(CLEANOBJS) $(CLEANFILES)

distclean:	clean
		@-$(RM) Makefile $(RUBY_EXTCONF_H) conftest.* mkmf.log
		@-$(RM) core ruby$(EXEEXT) *~ $(DISTCLEANFILES)

realclean:	distclean
install: install-so install-rb

install-so: $(RUBYARCHDIR)
install-so: $(RUBYARCHDIR)/$(DLLIB)
$(RUBYARCHDIR)/$(DLLIB): $(DLLIB)
	$(INSTALL_PROG) $(DLLIB) $(RUBYARCHDIR)
install-rb: pre-install-rb install-rb-default
install-rb-default: pre-install-rb-default
pre-install-rb: Makefile
pre-install-rb-default: Makefile
$(RUBYARCHDIR):
	$(MAKEDIRS) $@

site-install: site-install-so site-install-rb
site-install-so: install-so
site-install-rb: install-rb

.SUFFIXES: .c .m .cc .cxx .cpp .C .o

.cc.o:
	$(CXX) $(INCFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c $<

.cxx.o:
	$(CXX) $(INCFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c $<

.cpp.o:
	$(CXX) $(INCFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c $<

.C.o:
	$(CXX) $(INCFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c $<

.c.o:
	$(CC) $(INCFLAGS) $(CPPFLAGS) $(CFLAGS) -c $<

$(DLLIB): $(OBJS) Makefile
	@-$(RM) $@
	$(LDSHARED) -o $@ $(OBJS) $(LIBPATH) $(DLDFLAGS) $(LOCAL_LIBS) $(LIBS)



$(OBJS): ruby.h defines.h
