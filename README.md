TODO: Find a better name for this project than "buseds-hab"!

### [Components List](Components.md)

### [Block Diagram](Block-Diagram.md)

### Design Notes

The electronics collects data from the sensors and GPS. [Common GPS errors](http://ukhas.org.uk/guides:common_coding_errors_payload_testing).

The data is formatted as a
[UKHAS Telemetry String](http://ukhas.org.uk/communication:protocol)
and transmitted on 434.075MHz using a NTX2
transmitter. [Useful guide to using NTX2](http://ukhas.org.uk/guides:linkingarduinotontx2).
The Telemetry Strings are also logged to a µSD card.

Gets received by Yaesu FT790R.

Audio links to laptop, UKHAS's
[dl-fldigi](http://ukhas.org.uk/projects:dl-fldigi) is used to decode and upload.
