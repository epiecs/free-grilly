#include <string>
#include <Arduino.h>

String generate_hostname(String prefix);

// "grilly-plus-" + the first 8 hex characters of the grill uuid (dashes removed, lowercase),
// e.g. "grilly-plus-43c62ed2". Used for both the WiFi/mDNS hostname and the /api/grill response.
String compute_grill_hostname(String uuid);

// https://forum.arduino.cc/t/how-to-print-the-leading-zeroes/446702/17
// https://cplusplus.com/doc/oldtutorial/templates/
// templates need to be in the header; recipe, not a cake
// https://forum.arduino.cc/t/error-with-a-template-function-in-a-library/909719/9
template <typename byteType>
String byte_with_leading_0(byteType bytes, int split_every = 128, bool use_prefix = false)
{
    String formatted_representation = "";
    if (use_prefix)
    {
        formatted_representation += "0b";
    }

    // bitread returns 0/1 aka true/false
    for (int i = 0; i < 8 * sizeof(bytes); i++)
    {
        if (i > 0 && (i % split_every) == 0)
        {
            formatted_representation += " ";
        }

        formatted_representation += bitRead(bytes, sizeof(bytes) * 8 - i - 1) ? "1" : "0";
    }

    return formatted_representation;
}

// std::string get_timezone_code(std::string timezone);