#include <usb_names.h>

// Edit these lines to create your own name.  The length must
// match the number of characters in your custom name.

#define MIDI_NAME   {'I','c','a','r','u','s', ' ', 'M', 'I', 'D', 'I'}
#define MIDI_NAME_LEN  11

// Do not change this part.  This exact format is required by USB.

struct usb_string_descriptor_struct usb_string_product_name = {
        2 + MIDI_NAME_LEN * 2,
        3,
        MIDI_NAME
};