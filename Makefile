# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..

# FLAGS will be passed to both the C and C++ compiler
FLAGS = -Isrc -Ilib -Iview

# Add .cpp files to the build
SOURCES = $(wildcard src/*.cpp) \
		  $(wildcard src/model/*.cpp) \
		  $(wildcard src/util/*.cpp) \
		  $(wildcard src/view/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)

# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk


ifdef ARCH_WIN
# extra dist target for Azure CI Windows build, as there is only 7zip available and no zip command
azure-win-dist: all
	rm -rf dist
	mkdir -p dist/$(SLUG)
	@# Strip and copy plugin binary
	cp $(TARGET) dist/$(SLUG)/
	$(STRIP) -s dist/$(SLUG)/$(TARGET)
	@# Copy distributables
	cp -R $(DISTRIBUTABLES) dist/$(SLUG)/
	@# Create ZIP package
	cd dist && 7z a -tzip -mx=9 $(SLUG)-$(VERSION)-$(ARCH).zip -r $(SLUG)
endif
