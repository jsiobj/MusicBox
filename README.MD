## Tests of commuinication modes for PN532 / Elechouse module
### Elechouse Library

- I2C : Does not work with NeoTrellis although addresses seem to be different
- SPI : Does not work along with SD Card on VS1053 although they do have their own CS pins
- HSU : Works

No interrupt based method available in this lib.

### Adafruit library
Not working with Elechouse breakout

### Seeed
Not tested yet

