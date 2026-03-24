set(src_dirs
             OLED
             Menu_Cmd)

set(include_dirs
             OLED
             Menu_Cmd)
set(requires
             driver
             BSP)

idf_component_register(SRC_DIRS ${src_dirs} INCLUDE_DIRS ${include_dirs} REQUIRES ${requires})
component_compile_options(-ffast-math -O3 -Wno-error=format=-Wno-format)