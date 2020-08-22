idf_component_register(SRCS "main.c" "motor.c" "webserver.c"
					   INCLUDE_DIRS "."
					   EMBED_TXTFILES "template/skyline.html"
					   				  "static/manifest.json")