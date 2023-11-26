## Tests of commuinication modes for PN532 / Elechouse module

I'd like to add some RFID reader to be able to select. I am testing Elechouse PN532 reader.

### Elechouse Library

- I2C : Does not work with NeoTrellis although addresses seem to be different
- SPI : Does not work along with SD Card on VS1053 although they do have their own CS pins
- HSU : Works

No interrupt based method available in this lib.

### Adafruit library
Got HSU to work but seems there is no interrupt when using HSU
Could not get I2C nor SPI to work

### Seeed
Seeed lib is a mess...

### Conclusion
Forgot pull ups on I2C lines...
Works with Adafruit library

## TODO
### Config
Have a mode to test and configure
Keeping one key pressed at startup time get into maintenance mode

Use SD card to store default / startup config
- default brightness
- default volume
- keys config ?
