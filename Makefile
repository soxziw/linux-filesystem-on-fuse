.PHONY: all clean

build:
	@mkdir -p build
	@cd build && cmake .. > /dev/null 2>&1 && make build | grep -vE "make\[[0-9]+\]"

release:
	@mkdir -p build
	@cd build && cmake .. > /dev/null 2>&1 && make release | grep -vE "make\[[0-9]+\]"

clean:
	@mkdir -p build
	@cd build && cmake .. > /dev/null 2>&1 && make cleanup | grep -vE "make\[[0-9]+\]"
	@rm -rf build
