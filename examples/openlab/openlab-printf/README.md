openlab-printf
==============

Openlab comes with a non-standard, non-default implementation of printf.
This mileage provides possibly more precise floats printf, but does not
support e.g. %lu and %0.2f formats.

By default, openlab platforms use the standard printf implementation that
comes with gcc/glibc.  However, should you wish to use the openlab printf,
include the following in your project Makefile, as shown in this project.

```
OPENLAB_SRC += printf/printf.c printf/prints.c printf/printf_float.c
```

Note: switching from one to the other may require a ``make clean``
