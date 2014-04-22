all: 
	@cook -l build-log

run: all
	cd dist && ./bochs -q

clean:
	cook clean
