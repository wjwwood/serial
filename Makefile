all: serial

CMAKE_FLAGS := -DCMAKE_INSTALL_PREFIX=/tmp/usr/local

install:
	cd build && make install

uninstall:
	cd build && make uninstall

serial:
	@mkdir -p build
	cd build && cmake $(CMAKE_FLAGS) ..
ifneq ($(MAKE),)
	cd build && $(MAKE)
else
	cd build && make
endif

.PHONY: clean
clean:
	rm -rf build

.PHONY: doc
doc:
	@doxygen doc/Doxyfile
ifeq ($(UNAME),Darwin)
	@open doc/html/index.html
endif

.PHONY: test
test:
	@mkdir -p build
	cd build && cmake $(CMAKE_FLAGS) ..
ifneq ($(MAKE),)
	cd build && $(MAKE) run_tests
else
	cd build && make run_tests
endif
