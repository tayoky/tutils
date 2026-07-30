TOP = $(CURDIR)
TMAKE_DIR = $(TOP)/make
include $(TMAKE_DIR)/tmake-init.mk
include $(TMAKE_DIR)/tmake-version.mk

DOCS = COPYING.txt README.md
DOCSDIR = $(DOCDIR)/tutils
FILESGROUPS += DOCS

PACKAGE = tutils
SRCS = $(wildcard src/*.c)
CFLAGS += -Iinclude

include $(TMAKE_DIR)/tmake-prog.mk
include $(TMAKE_DIR)/tmake-locale.mk

# list of all commands
# keep this in alphabetic order
CMDS = [ \
	basename \
	cat \
	chmod \
	chown \
	clear \
	cp \
	dd \
	dirname \
	echo \
	false \
	find \
	gunzip \
	gzip \
	head \
	hex \
	id \
	install \
	ln \
	ls \
	mkdir \
	mv \
	pwd \
	readlink \
	realpath \
	rm \
	rmdir \
	seq \
	sleep \
	stat \
	tee \
	test \
	touch \
	tr \
	true \
	unlink \
	wc \
	which \
	whoami \
	xargs\
	yes \
	zcat

.PHONY : install-symlinks
install : install-symlinks
install-symlinks : install-bin
	$(Q)for CMD in $(CMDS) ; do \
		echo "LN_S tutils $(DESTDIR)$(BINDIR)/$$CMD" ; \
		$(LN_S) -f "tutils" "$(DESTDIR)$(BINDIR)/$$CMD" ; \
	done
