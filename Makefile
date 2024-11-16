.PHONY: all clean

build:
	@mkdir -p build
	@cmake -B build -S .
	@make -C build build | grep -vE "make\[[0-9]+\]"

release:
	@mkdir -p build
	@cmake -B build -S .
	@make -C build release | grep -vE "make\[[0-9]+\]"

clean:
	@mkdir -p build
	@cmake -B build -S . > /dev/null 2>&1
	@make -C build cleanup | grep -vE "make\[[0-9]+\]"
	@rm -rf build

