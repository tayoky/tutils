TOP = $(CURDIR)
TMAKE_DIR = $(TOP)/make
include $(TMAKE_DIR)/tmake-init.mk
include $(TMAKE_DIR)/tmake-version.mk

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
install-symlinks :
	$(Q)for CMD in $(CMDS) ; do \
		echo "LN_S tutils $(DESTDIR)$(PREFIX)/bin/$$CMD" ; \
		$(LN_S) -f "tutils" "$(DESTDIR)$(PREFIX)/bin/$$CMD" ; \
	done

FILES = COPYING.txt README.md
FILESDIR = $(DOCDIR)/tutils
include $(TMAKE_DIR)/tmake-files.mk
