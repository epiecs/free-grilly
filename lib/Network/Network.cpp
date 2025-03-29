#include <Network.h>

#include <WiFi.h>

#include "Global.h"

constexpr int CONNECT_TIMEOUT_SECONDS = 15;

void start_local_ap(String ssid, String password, IPAddress ip, IPAddress subnet, IPAddress gateway)
{
    const char *local_password = NULL;

    if (password != "")
    {
        local_password = password.c_str();
    }

    // Start local ap
    Serial.println("Starting local wifi ap");
    WiFi.softAPConfig(ip, gateway, subnet);
    WiFi.softAP(ssid.c_str(), local_password);
    Serial.printf("Local SSID: %s \n", ssid.c_str());
    Serial.printf("Local IP: %s \n", WiFi.softAPIP().toString().c_str());
}

bool connect_to_wifi(String ssid, String password, bool static_ip, IPAddress ip, IPAddress subnet, IPAddress gateway, IPAddress dns1, IPAddress dns2)
{
    int timeout = CONNECT_TIMEOUT_SECONDS * 1000;
    int step = 500;

    Serial.printf("Connecting to wifi SSID: %s \n", ssid.c_str());
    WiFi.begin(ssid.c_str(), password.c_str());

    int expired_time = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(step);
        expired_time += step;

        if (expired_time > timeout)
        {
            Serial.println("");
            Serial.println("Failed to connect to WiFi");
            return false;
        }
    }

    Serial.println("");

    if (static_ip)
    {
        if (!WiFi.config(ip, gateway, subnet, dns1, dns2))
        {
            Serial.println("Failed to configure Static IP");
        }
        else
        {
            Serial.println("Static IP configured!");
        }
    }

    return true;
}

void event_wifi_connected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    wifi_connected = true;
    
    Serial.println("Connected to wifi");
    print_wifi_connection();
}

void event_wifi_ip_acquired(WiFiEvent_t event, WiFiEventInfo_t info)
{
    wifi_ip = WiFi.localIP();
    
    Serial.println("Received IP");
    print_wifi_connection();
}

void event_wifi_disconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    wifi_connected = false;

    Serial.println("Wifi disconnected");

    Serial.print("Reason: ");
    Serial.println(get_wifi_error_status(info.wifi_sta_disconnected.reason).c_str());

    Serial.println("Trying to reconnect");
    WiFi.reconnect();
}

void print_wifi_connection()
{
    // String rssi = String(WiFi.RSSI());

    Serial.printf("Connected to wifi SSID: %s \n", WiFi.SSID().c_str());
    Serial.printf("IP:      %s \n", WiFi.localIP().toString().c_str());
    Serial.printf("Gateway: %s \n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("Subnet:  %s \n", WiFi.subnetMask().toString().c_str());
    Serial.printf("DNS:     %s \n", WiFi.dnsIP(0).toString().c_str());
    Serial.printf("DNS 2:   %s \n", WiFi.dnsIP(1).toString().c_str());
    Serial.printf("RSSI:    %d \n", WiFi.RSSI());
}

String get_wifi_connection_status(int statuscode)
{
    switch (statuscode){
        case ARDUINO_EVENT_WIFI_READY:                return "WiFi interface ready"; break;
        case ARDUINO_EVENT_WIFI_SCAN_DONE:            return "Completed scan for access points"; break;
        case ARDUINO_EVENT_WIFI_STA_START:            return "WiFi client started"; break;
        case ARDUINO_EVENT_WIFI_STA_STOP:             return "WiFi clients stopped"; break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:        return "Connected to access point"; break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:     return "Disconnected from WiFi access point"; break;
        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:  return "Authentication mode of access point has changed"; break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:           return "Obtained IP address: "; break;
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:          return "Lost IP address and IP address is reset to 0"; break;
        case ARDUINO_EVENT_WPS_ER_SUCCESS:            return "WiFi Protected Setup (WPS): succeeded in enrollee mode"; break;
        case ARDUINO_EVENT_WPS_ER_FAILED:             return "WiFi Protected Setup (WPS): failed in enrollee mode"; break;
        case ARDUINO_EVENT_WPS_ER_TIMEOUT:            return "WiFi Protected Setup (WPS): timeout in enrollee mode"; break;
        case ARDUINO_EVENT_WPS_ER_PIN:                return "WiFi Protected Setup (WPS): pin code in enrollee mode"; break;
        case ARDUINO_EVENT_WIFI_AP_START:             return "WiFi access point started"; break;
        case ARDUINO_EVENT_WIFI_AP_STOP:              return "WiFi access point  stopped"; break;
        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:      return "Client connected"; break;
        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:   return "Client disconnected"; break;
        case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:     return "Assigned IP address to client"; break;
        case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:    return "Received probe request"; break;
        case ARDUINO_EVENT_WIFI_AP_GOT_IP6:           return "AP IPv6 is preferred"; break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:          return "STA IPv6 is preferred"; break;
        case ARDUINO_EVENT_ETH_GOT_IP6:               return "Ethernet IPv6 is preferred"; break;
        case ARDUINO_EVENT_ETH_START:                 return "Ethernet started"; break;
        case ARDUINO_EVENT_ETH_STOP:                  return "Ethernet stopped"; break;
        case ARDUINO_EVENT_ETH_CONNECTED:             return "Ethernet connected"; break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:          return "Ethernet disconnected"; break;
        case ARDUINO_EVENT_ETH_GOT_IP:                return "Obtained IP address"; break;
        default:                                      return "Unknown status code"; break;
    }
}

String get_wifi_error_status(int statuscode)
{
    switch (statuscode){
        case WIFI_REASON_UNSPECIFIED:                        return "Unspecified reason"; break;
        case WIFI_REASON_AUTH_EXPIRE:                        return "Authentication expired"; break;
        case WIFI_REASON_AUTH_LEAVE:                         return "Deauthentication due to leaving"; break;
        case WIFI_REASON_ASSOC_EXPIRE:                       return "Deprecated, will be removed in next IDF major release"; break;
        // case WIFI_REASON_DISASSOC_DUE_TO_INACTIVITY:         return "Disassociated due to inactivity"; break;
        case WIFI_REASON_ASSOC_TOOMANY:                      return "Too many associated stations"; break;
        case WIFI_REASON_NOT_AUTHED:                         return "Deprecated, will be removed in next IDF major release"; break;
        // case WIFI_REASON_CLASS2_FRAME_FROM_NONAUTH_STA:      return "Class 2 frame received from nonauthenticated STA"; break;
        case WIFI_REASON_NOT_ASSOCED:                        return "Deprecated, will be removed in next IDF major release"; break;
        // case WIFI_REASON_CLASS3_FRAME_FROM_NONASSOC_STA:     return "Class 3 frame received from nonassociated STA"; break;
        case WIFI_REASON_ASSOC_LEAVE:                        return "Deassociated due to leaving"; break;
        case WIFI_REASON_ASSOC_NOT_AUTHED:                   return "Association but not authenticated"; break;
        case WIFI_REASON_DISASSOC_PWRCAP_BAD:                return "Disassociated due to poor power capability"; break;
        case WIFI_REASON_DISASSOC_SUPCHAN_BAD:               return "Disassociated due to unsupported channel"; break;
        case WIFI_REASON_BSS_TRANSITION_DISASSOC:            return "Disassociated due to BSS transition"; break;
        case WIFI_REASON_IE_INVALID:                         return "Invalid Information Element (IE)"; break;
        case WIFI_REASON_MIC_FAILURE:                        return "MIC failure"; break;
        case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:             return "4-way handshake timeout"; break;
        case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:           return "Group key update timeout"; break;
        case WIFI_REASON_IE_IN_4WAY_DIFFERS:                 return "IE differs in 4-way handshake"; break;
        case WIFI_REASON_GROUP_CIPHER_INVALID:               return "Invalid group cipher"; break;
        case WIFI_REASON_PAIRWISE_CIPHER_INVALID:            return "Invalid pairwise cipher"; break;
        case WIFI_REASON_AKMP_INVALID:                       return "Invalid AKMP"; break;
        case WIFI_REASON_UNSUPP_RSN_IE_VERSION:              return "Unsupported RSN IE version"; break;
        case WIFI_REASON_INVALID_RSN_IE_CAP:                 return "Invalid RSN IE capabilities"; break;
        case WIFI_REASON_802_1X_AUTH_FAILED:                 return "802.1X authentication failed"; break;
        case WIFI_REASON_CIPHER_SUITE_REJECTED:              return "Cipher suite rejected"; break;
        case WIFI_REASON_TDLS_PEER_UNREACHABLE:              return "TDLS peer unreachable"; break;
        case WIFI_REASON_TDLS_UNSPECIFIED:                   return "TDLS unspecified"; break;
        case WIFI_REASON_SSP_REQUESTED_DISASSOC:             return "SSP requested disassociation"; break;
        case WIFI_REASON_NO_SSP_ROAMING_AGREEMENT:           return "No SSP roaming agreement"; break;
        case WIFI_REASON_BAD_CIPHER_OR_AKM:                  return "Bad cipher or AKM"; break;
        case WIFI_REASON_NOT_AUTHORIZED_THIS_LOCATION:       return "Not authorized in this location"; break;
        case WIFI_REASON_SERVICE_CHANGE_PERCLUDES_TS:        return "Service change precludes TS"; break;
        case WIFI_REASON_UNSPECIFIED_QOS:                    return "Unspecified QoS reason"; break;
        case WIFI_REASON_NOT_ENOUGH_BANDWIDTH:               return "Not enough bandwidth"; break;
        case WIFI_REASON_MISSING_ACKS:                       return "Missing ACKs"; break;
        case WIFI_REASON_EXCEEDED_TXOP:                      return "Exceeded TXOP"; break;
        case WIFI_REASON_STA_LEAVING:                        return "Station leaving"; break;
        case WIFI_REASON_END_BA:                             return "End of Block Ack (BA)"; break;
        case WIFI_REASON_UNKNOWN_BA:                         return "Unknown Block Ack (BA)"; break;
        case WIFI_REASON_TIMEOUT:                            return "Timeout"; break;
        case WIFI_REASON_PEER_INITIATED:                     return "Peer initiated disassociation"; break;
        case WIFI_REASON_AP_INITIATED:                       return "AP initiated disassociation"; break;
        case WIFI_REASON_INVALID_FT_ACTION_FRAME_COUNT:      return "Invalid FT action frame count"; break;
        case WIFI_REASON_INVALID_PMKID:                      return "Invalid PMKID"; break;
        case WIFI_REASON_INVALID_MDE:                        return "Invalid MDE"; break;
        case WIFI_REASON_INVALID_FTE:                        return "Invalid FTE"; break;
        case WIFI_REASON_TRANSMISSION_LINK_ESTABLISH_FAILED: return "Transmission link establishment failed"; break;
        case WIFI_REASON_ALTERATIVE_CHANNEL_OCCUPIED:        return "Alternative channel occupied"; break;
        case WIFI_REASON_BEACON_TIMEOUT:                     return "Beacon timeout"; break;
        case WIFI_REASON_NO_AP_FOUND:                        return "No AP found"; break;
        case WIFI_REASON_AUTH_FAIL:                          return "Authentication failed"; break;
        case WIFI_REASON_ASSOC_FAIL:                         return "Association failed"; break;
        case WIFI_REASON_HANDSHAKE_TIMEOUT:                  return "Handshake timeout"; break;
        case WIFI_REASON_CONNECTION_FAIL:                    return "Connection failed"; break;
        case WIFI_REASON_AP_TSF_RESET:                       return "AP TSF reset"; break;
        case WIFI_REASON_ROAMING:                            return "Roaming"; break;
        case WIFI_REASON_ASSOC_COMEBACK_TIME_TOO_LONG:       return "Association comeback time too long"; break;
        case WIFI_REASON_SA_QUERY_TIMEOUT:                   return "SA query timeout"; break;
        // case WIFI_REASON_NO_AP_FOUND_W_COMPATIBLE_SECURITY:  return "No AP found with compatible security"; break;
        // case WIFI_REASON_NO_AP_FOUND_IN_AUTHMODE_THRESHOLD:  return "No AP found in auth mode threshold"; break;
        // case WIFI_REASON_NO_AP_FOUND_IN_RSSI_THRESHOLD:      return "No AP found in RSSI threshold"; break;
        default:                                             return "Unknown status code"; break;
    }
}
