# makefile include to manage .mo/.po/.pot files

LOCALES ?= $(wildcard locale/*.po)
TEMPLATE_POT ?= $(BUILDDIR)/locale/template.pot
LOCALES_MO ?= $(LOCALES:%.po=$(BUILDDIR)/%.mo)

ifneq ($(MULTI_LANGUAGES),no)
ifneq ($(strip $(LOCALES_MO)),)
all : $(LOCALES_MO)
$(BUILDDIR)/%.mo : %.po
	@mkdir -p "$(@D)"
	@echo "GEN $@"
	$(Q)msgfmt -o "$@" $^

.PHONY : install-locale
install : install-locale
install-locale : $(LOCALES)
	@mkdir -p "$(LOCALEDIR)/$(PACKAGE)"
	@echo "INSTALL $(LOCALES_MO)"
	@cp $(LOCALES_MO) "$(LOCALEDIR)/$(PACKAGE)"

.PHONY : uninstall-locale
uninstall : uninstall-locale
uninstall-locale :
	@echo "UNINSTALL $(LOCALEDIR)/$(PACKAGE)"
	@rm -fr "$(LOCALEDIR)/$(PACKAGE)"
endif

.PHONY : update-po
update-po : $(TEMPLATE_POT)
$(TEMPLATE_POT) : $(SRCS)
	@mkdir -p "$(@D)"
	@echo "GEN $(TEMPLATE_POT)"
	$(Q)xgettext --keyword=_ \
		--package-name="$(PACKAGE)" \
		--package-version="$(VERSION)" \
		-o "$@" $^
	$(Q)for I in $(LOCALES) ; do \
		msgmerge --update $$I "$@"; \
	done
endif
