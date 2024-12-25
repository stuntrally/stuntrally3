include(BundleUtilities)

if (APPLE)
  set(PLUGIN_EXTENSION "dylib")
elseif (WIN32)
  set(PLUGIN_EXTENSION "dll")
else()
  set(PLUGIN_EXTENSION "so")
endif()

file(GLOB PLUGINS "${LIB_DIR}/Plugin_*.${PLUGIN_EXTENSION}")
file(GLOB RENDERSYSTEMS "${LIB_DIR}/RenderSystem_*.${PLUGIN_EXTENSION}")

set(EXTRA_LIBS ${PLUGINS} ${RENDERSYSTEMS})
fixup_bundle("${EXECUTABLE}" "${EXTRA_LIBS}" "${CONAN_BIN_DIRS}")