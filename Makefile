all:
	pio run -t upload

build:
	pio run

get_devices:
	pio device list

connector:
	pio run -e connector -t upload

member:
	pio run -e member -t upload && pio device monitor -e member

connector32:
	pio run -e connector32 -t upload && pio device monitor -e connector32

monitor:
	pio device monitor