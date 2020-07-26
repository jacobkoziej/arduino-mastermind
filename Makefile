PORT ?=

all:
	arduino-cli compile --verbose --fqbn arduino:avr:uno ./mastermind

clean:
	rm -vrf ./mastermind/build

flash: all
ifndef PORT
	$(error variable: [PORT] (not defined))
endif
	arduino-cli upload --verbose --verify --port $(PORT) --fqbn arduino:avr:uno ./mastermind

.PHONY: all clean flash
