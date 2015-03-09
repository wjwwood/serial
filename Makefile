all: serial

CMAKE_FLAGS := -DCMAKE_INSTALL_PREFIX=/tmp/usr/local
UNAME := $(shell uname -s)

install_deps:
ifeq ($(UNAME),Darwin)
	brew tap ros/deps
	brew update
	brew outdated boost || brew upgrade boost || brew install boost
	sudo pip install rosinstall_generator wstool rosdep empy catkin_pkg
	sudo rosdep init
	rosdep update
	mkdir catkin_ws
	cd catkin_ws && rosinstall_generator catkin --rosdistro hydro --tar > catkin.rosinstall
	cd catkin_ws && wstool init src catkin.rosinstall
	cd catkin_ws && rosdep install --from-paths src --ignore-src -y
	cd catkin_ws && ./src/catkin/bin/catkin_make install
	echo "source catkin_ws/install/setup.bash" > setup.bash
else
	sudo sh -c 'echo "deb http://packages.ros.org/ros/ubuntu precise main" > /etc/apt/sources.list.d/ros-latest.list'
	wget http://packages.ros.org/ros.key -O - | sudo apt-key add -
	sudo apt-get update
	sudo apt-get install ros-hydro-catkin libboost-dev
	echo "source /opt/ros/hydro/setup.bash" > setup.bash
endif

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
