list(APPEND OPTIONS
        -Dmmx=disabled
        -Dsse2=disabled
        -Dssse3=disabled
        -Da64-neon=disabled
        -Darm-simd=disabled
        -Dneon=disabled
)

vcpkg_from_gitlab(
    OUT_SOURCE_PATH SOURCE_PATH
    GITLAB_URL https://gitlab.freedesktop.org
    REPO pixman/pixman
    REF "pixman-${VERSION}"
    SHA512 daeb25d91e9cb8d450a6f050cbec1d91e239a03188e993ceb6286605c5ed33d97e08d6f57efaf1d5c6a8a1eedb1ebe6c113849a80d9028d5ea189c54601be424
    PATCHES
        no-host-cpu-checks.patch
        fix_clang-cl.patch
        missing_intrin_include.patch
)

# Meson install wrongly pkgconfig file!
vcpkg_configure_meson(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS ${OPTIONS}
        -Ddemos=disabled
        -Dgtk=disabled
        -Dlibpng=enabled
        -Dtests=disabled
)
vcpkg_install_meson()
vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

# # Handle copyright
file(INSTALL "${SOURCE_PATH}/COPYING" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
file(INSTALL "${SOURCE_PATH}/README" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME readme.txt)
