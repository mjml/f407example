usbexample
==========

This project provides a simple CDC device that you can connect a terminal to and issue commands.

The device offers a terminal server that will respond to simple commands: PRINT, WRITE, and READ.

PRINT simply echoes a linefeed-terminated string back. Strings up to 512 characters are allowed. If you exceed 512 bytes, your input will be truncated.

WRITE stores a string in the server's flash memory. Again, strings up to 512 characters are allowed.

READ prints a stored string from the server's memory.

