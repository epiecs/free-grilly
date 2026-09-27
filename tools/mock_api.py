"""Mock grill API for tools/dev_server.py --mock: realistic data for development and screenshots."""
import json
import time

START = time.time()

SETTINGS = {
    "name": "Big Green Egg", "uuid": "43c62ed2-4dc0-41a5-8f71-16db60155739", "firmware_version": "26.09.27",
    "temperature_unit": "celcius", "beep_enabled": True, "beep_volume": 4, "beep_degrees_before": 5,
    "beep_outside_target": True, "beep_on_ready": True, "cucaracha_enabled": False,
    "screen_timeout_minutes": 0, "backlight_timeout_minutes": 5, "backlight_brightness": 4,
    "opengrill_server": "", "mqtt_broker": "192.168.1.10", "mqtt_port": 1883, "mqtt_topic": "grilly-plus",
    "mqtt_user": "grill", "mqtt_password_set": True,
    "wifi_ssid": "HomeNet", "wifi_ip": "0.0.0.0", "wifi_subnet": "0.0.0.0", "wifi_gateway": "0.0.0.0",
    "wifi_dns": "0.0.0.0", "wifi_password_set": True,
    "local_ap_ssid": "GrillyPlus_A1B2C3", "local_ap_ip": "192.168.200.10", "local_ap_subnet": "255.255.255.0",
    "local_ap_gateway": "192.168.200.10", "local_ap_password_set": True, "admin_password_set": False,
}

PROBES = [
    {"probe_id": 1, "name": "Brisket", "target_temperature": 95.0, "minimum_temperature": 0.0, "connected": True,
     "probe_type": "grilleye_iris", "reference_kohm": 100, "reference_celcius": 25, "reference_beta": 4250},
    {"probe_id": 2, "name": "Ribs", "target_temperature": 93.0, "minimum_temperature": 88.0, "connected": True,
     "probe_type": "grilleye_iris", "reference_kohm": 100, "reference_celcius": 25, "reference_beta": 4250},
    {"probe_id": 3, "name": "Grill", "target_temperature": 0.0, "minimum_temperature": 0.0, "connected": True,
     "probe_type": "maverick_et733", "reference_kohm": 200, "reference_celcius": 25, "reference_beta": 4250},
] + [
    {"probe_id": n, "name": "Probe %d" % n, "target_temperature": 0.0, "minimum_temperature": 0.0, "connected": False,
     "probe_type": "grilleye_iris", "reference_kohm": 100, "reference_celcius": 25, "reference_beta": 4250}
    for n in range(4, 9)
]

CONNECTED_AT = {1: START - 18720, 2: START - 13200, 3: START - 19200}
BASE_TEMPERATURE = {1: 71.4, 2: 90.2, 3: 118.0}


def temperature(probe):
    if not probe["connected"]:
        return 0.0
    drift = (time.time() - START) / 600.0
    return round(BASE_TEMPERATURE[probe["probe_id"]] + drift, 1)


def grill():
    return {
        "name": SETTINGS["name"], "unique_id": SETTINGS["uuid"], "firmware_version": SETTINGS["firmware_version"],
        "battery_percentage": 82, "battery_charging": True, "wifi_connected": True, "wifi_ssid": SETTINGS["wifi_ssid"],
        "wifi_ip": "192.168.1.50", "wifi_signal": -58, "temperature_unit": SETTINGS["temperature_unit"],
        "probes": [
            {"probe_id": p["probe_id"], "name": p["name"], "temperature": temperature(p),
             "minimum_temperature": p["minimum_temperature"], "target_temperature": p["target_temperature"],
             "connected": p["connected"],
             "connected_seconds": int(time.time() - CONNECTED_AT[p["probe_id"]]) if p["connected"] else 0}
            for p in PROBES
        ],
    }


def probes():
    return [dict(p, temperature=temperature(p)) for p in PROBES]


WIFI_SCAN = [
    {"ssid": "HomeNet", "signal_strength": -52, "auth_method": "wpa2_psk"},
    {"ssid": "HomeNet-Guest", "signal_strength": -61, "auth_method": "wpa2_psk"},
    {"ssid": "Neighbours", "signal_strength": -78, "auth_method": "wpa2_wpa3_psk"},
    {"ssid": "Cafe Free WiFi", "signal_strength": -86, "auth_method": "open"},
]


def handle(method, path, body):
    """Returns (status, json_body) for an /api request."""
    if method == "GET" and path == "/api/grill":
        return 200, grill()
    if method == "GET" and path == "/api/probes":
        return 200, probes()
    if method == "POST" and path == "/api/probes":
        for update in json.loads(body or b"[]"):
            probe = next((p for p in PROBES if p["probe_id"] == int(update["probe_id"])), None)
            if probe is None:
                return 400, {"error": "probe_id should be between 1 and 8"}
            probe.update({k: v for k, v in update.items() if k != "probe_id"})
        return 200, probes()
    if method == "GET" and path == "/api/settings":
        return 200, SETTINGS
    if method == "POST" and path == "/api/settings":
        update = json.loads(body or b"{}")
        password = update.get("local_ap_password")
        if password and len(password) < 8:
            return 400, {"error": "local_ap_password should be empty or at least 8 characters"}
        for key, value in update.items():
            if key.endswith("_password"):
                SETTINGS[key + "_set"] = value != ""
            else:
                SETTINGS[key] = value
        return 200, SETTINGS
    if method == "GET" and path == "/api/wifiscan":
        time.sleep(1.5)
        return 200, WIFI_SCAN
    if method == "POST" and path == "/api/update":
        time.sleep(2)
        return 200, {"success": True}
    return 404, {"error": "Not found"}
