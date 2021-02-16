PREFIX = /usr/local
LIBDIR = lib
INSTALL = install
SED = sed

install: all
	$(INSTALL) -m 0755 -d '$(DESTDIR)$(PREFIX)/include/AMF'
	$(INSTALL) -m 0755 -d '$(DESTDIR)$(PREFIX)/include/AMF/core'
	$(INSTALL) -m 0755 -d '$(DESTDIR)$(PREFIX)/include/AMF/components'
	$(INSTALL) -m 0644 amf/public/include/core/*.h '$(DESTDIR)$(PREFIX)/include/AMF/core'
	$(INSTALL) -m 0644 amf/public/include/components/*.h '$(DESTDIR)$(PREFIX)/include/AMF/components'

uninstall:
	rm -rf '$(DESTDIR)$(PREFIX)/include/AMF'

.PHONY: all install uninstall

