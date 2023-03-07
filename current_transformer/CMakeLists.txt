idf_component_register(
    SRCS "current_transformer.c"
    INCLUDE_DIRS "include"
)

idf_build_get_property(project_dir PROJECT_DIR)
set(EXTRA_COMPONENT_DIRS ${project_dir}/components)
include_directories(
    ${EXTRA_COMPONENT_DIRS}/driver
    ${EXTRA_COMPONENT_DIRS}/esp_adc_cal
)