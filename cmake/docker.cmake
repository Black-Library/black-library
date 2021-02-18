macro(add_app_docker_targets)
    add_custom_target(bl-app-builder
        COMMAND docker build --force-rm --tag bl/app/builder --file tools/docker/bl_app_builder.docker .
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Building bl-app-builder."
        VERBATIM)
endmacro()
