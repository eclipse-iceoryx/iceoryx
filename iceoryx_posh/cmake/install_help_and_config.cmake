install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/FAQ.md
	DESTINATION share/doc/iceoryx
	COMPONENT dev)

if(TOML_CONFIG)
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/etc/iceoryx/roudi_config.toml
		DESTINATION etc/
		COMPONENT dev)
endif(TOML_CONFIG)
