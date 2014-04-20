all: 
	@cook

run: all
	cd exec && ./bochs -q
