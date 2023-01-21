idf_component_register(
                    SRCS "main.c"
                         "../application/beep.c"
                         "../bsp/https_request.c"
                         "../bsp/wifi.c"
                         "../bsp/http_request.c"
                         "../bsp/uc8151d.c"
                         "../bsp/myfont.c"

                    INCLUDE_DIRS "."
                                 "../application"
                                 "../bsp"

                    # EMBED_TXTFILES server_root_cert.pem
                    )
                    
                    spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)
