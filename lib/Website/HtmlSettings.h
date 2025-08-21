const char HTML_SETTINGS[] = R"=====(
<!doctype html>
<html lang="en">
    <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title>Free Grilly</title>
        
        <link rel="stylesheet" type="text/css" href="custom-boostrap.min.css">
        
        <link rel="icon" href="data:,">
    </head>
  <body>
    <nav class="navbar navbar-expand bg-body-tertiary">
        <div class="container-fluid">
            <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#navbarNav" aria-controls="navbarNav" aria-expanded="false" aria-label="Toggle navigation">
                <span class="navbar-toggler-icon"></span>
            </button>
            <div class="collapse navbar-collapse" id="navbarNav">
                <ul class="navbar-nav">
                    <li class="nav-item">
                        <a class="nav-link" aria-current="page" href="/">Temperatures</a>
                    </li>
                    <li class="nav-item">
                        <a class="nav-link" href="/probes">Probes</a>
                    </li>
                    <li class="nav-item">
                        <a class="nav-link active" href="/settings">Settings</a>
                    </li>
                    <li class="nav-item">
                        <a class="nav-link" href="/about">About</a>
                    </li>
                    <li class="nav-item">
                        <a class="nav-link" href="/update">Update</a>
                    </li>
                </ul>
            </div>
        </div>
    </nav>
    <div class="container">
        <div class="row">
            <div class="my-2 col-sm-12">
                <div id="alert" class="alert alert-dismissible fade show" style="display: none;" role="alert">
                    <span id="alert-text"></span>
                    <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
                </div>
            </div>
        </div>
        
        
        <div class="row mt-2">
            <h5>Grill</h5>
        </div>
        <div class="row">
            <div class="row mt-2">
                <label for="grill_name" class="col-sm-2 col-form-label">Name</label>
                <div class="col-sm-10">
                    <input type="text" class="form-control" id="grill_name">
                </div>
            </div>
        </div>


        <div class="row mt-4">
            <h5>Temperature</h5>
        </div>
        <div class="row mt-2">
            <div class="row mt-2">
                <label for="temperature_unit" class="col-sm-2 col-form-label">Temperature unit</label>
                <div class="col-sm-10">
                    <select id="temperature_unit" class="form-select probe-type">
                        <option value="celcius">Celcius</option>
                        <option value="fahrenheit">Fahrenheit</option>
                    </select>
                </div>
            </div>
        </div>
        
        
        <div class="row mt-4">
            <h5>Sound</h5>
        </div>
        <div class="row mt-2">
            <div class="row mt-2">
                <label for="beep_enabled" class="col-sm-2 col-form-label">Beep enabled</label>
                <div class="col-sm-10">
                    <div class="form-check form-switch mt-2">
                        <input id="beep_enabled" class="form-check-input" type="checkbox">  
                    </div>
                </div>
            </div>

            <div class="row mt-2">
                <label for="beep_volume" class="col-sm-2 col-form-label">Beep volume</label>
                <div class="col-sm-10">
                    <select id="beep_volume" class="form-select probe-type">
                        <option value="0">0</option>
                        <option value="1">1</option>
                        <option value="2">2</option>
                        <option value="3">3</option>
                        <option value="4">4</option>
                        <option value="5">5</option>
                    </select>
                </div>
            </div>

            <div class="row mt-2">
                <label for="beep_degrees_before" class="col-sm-2 col-form-label">Beep degrees before</label>
                <div class="col-sm-10">
                    <select id="beep_degrees_before" class="form-select probe-type">
                        <option value="0">0</option>
                        <option value="5">5</option>
                        <option value="10">10</option>
                        <option value="15">15</option>
                    </select>
                </div>
            </div>

            <div class="row mt-2">
                <label for="beep_outside_target" class="col-sm-2 col-form-label">Beep outside target</label>
                <div class="col-sm-10">
                    <div class="form-check form-switch mt-2">
                        Beep when outside the target temperature
                        <input id="beep_outside_target" class="form-check-input" type="checkbox">  
                    </div>
                </div>
            </div>

            <div class="row mt-2">
                <label for="beep_on_ready" class="col-sm-2 col-form-label">Beep when ready</label>
                <div class="col-sm-10">
                    <div class="form-check form-switch mt-2">
                        <input id="beep_on_ready" class="form-check-input" type="checkbox">  
                    </div>
                </div>
            </div>
        </div>
        
        <div class="row mt-4">
            <h5>Screen</h5>
        </div>
        <div class="row mt-2">

            <div class="row mt-2">
                <label for="screen_timeout_minutes" class="col-sm-2 col-form-label">Screen timeout (minutes)</label>
                <div class="col-sm-10">
                    <select id="screen_timeout_minutes" class="form-select">
                        <option value="0">Disabled</option>
                        <option value="1">1</option>
                        <option value="5">5</option>
                        <option value="10">10</option>
                    </select>
                </div>
            </div>

            <div class="row mt-2">
                <label for="backlight_timeout_minutes" class="col-sm-2 col-form-label">Backlight timeout (minutes)</label>
                <div class="col-sm-10">
                    <select id="backlight_timeout_minutes" class="form-select">
                        <option value="0">Disabled</option>
                        <option value="1">1</option>
                        <option value="5">5</option>
                        <option value="10">10</option>
                    </select>
                </div>
            </div>

            <div class="row mt-2">
                <label for="backlight_brightness" class="col-sm-2 col-form-label">Backlight brightness</label>
                <div class="col-sm-10">
                    <select id="backlight_brightness" class="form-select">
                        <option value="0">0</option>
                        <option value="1">1</option>
                        <option value="2">2</option>
                        <option value="3">3</option>
                        <option value="4">4</option>
                        <option value="5">5</option>
                    </select>
                </div>
            </div>

        </div>
        
        <div class="row mt-4">
            <h5>Wifi</h5>
        </div>
        <div class="row mt-2">
            <label for="wifi_ssid" class="col-sm-2 col-form-label">Wifi SSID</label>
            <div class="col-sm-10">
                <input type="text" class="form-control" id="wifi_ssid">
            </div>
        </div>
        
        <div class="row mt-2">
            <label for="wifi_password" class="col-sm-2 col-form-label"></label>
            <div class="col-8 col-sm-6">
                <select id="wifi_scan_results" class="form-select" disabled>
                    <option value="manual">Choose your network</option>
                </select>
            </div>
            <div class="col-4 col-sm-4">
                <div class="form-check form-switch float-end">
                    <button id="wifi_scan" type="button" class="btn btn-primary" disabled>Scan</button>
                </div>
            </div>
        </div>

        <div class="row mt-2">
            <label for="wifi_password" class="col-sm-2 col-form-label">Wifi password</label>
            <div class="col-sm-10">
                <input type="password" class="form-control" id="wifi_password">
            </div>
        </div>

        <div class="row mt-2">
            <label for="wifi_ip" class="col-sm-2 col-form-label">Wifi IP</label>
            <div class="col-sm-10">
                <input type="text" class="form-control" id="wifi_ip">
            </div>
        </div>
        
        <div class="row mt-2">
            <label for="wifi_subnet" class="col-sm-2 col-form-label">Wifi subnet</label>
            <div class="col-sm-10">
                <input type="text" class="form-control" id="wifi_subnet">
            </div>
        </div>
        
        <div class="row mt-2">
            <label for="wifi_gateway" class="col-sm-2 col-form-label">Wifi gateway</label>
            <div class="col-sm-10">
                <input type="text" class="form-control" id="wifi_gateway">
            </div>
        </div>

        <div class="row mt-2">
            <label for="wifi_dns" class="col-sm-2 col-form-label">Wifi dns</label>
            <div class="col-sm-10">
                <input type="text" class="form-control" id="wifi_dns">
            </div>
        </div>
        

        <div class="row mt-4">
            <h5>Local AP</h5>
        </div>
        <div class="row mt-2">
            <label for="local_ap_ssid" class="col-sm-2 col-form-label">Local AP SSID</label>
            <div class="col-sm-10">
                <input type="text" class="form-control" id="local_ap_ssid">
            </div>
        </div>
        
        <div class="row mt-2">
            <label for="local_ap_password" class="col-sm-2 col-form-label">Local AP password</label>
            <div class="col-sm-10">
                <input type="password" class="form-control" id="local_ap_password">
            </div>
        </div>

        <div class="row mt-2">
            <label for="local_ap_ip" class="col-sm-2 col-form-label">Local AP IP</label>
            <div class="col-sm-10">
                <input type="text" class="form-control" id="local_ap_ip">
            </div>
        </div>
        
        <div class="row mt-2">
            <label for="local_ap_subnet" class="col-sm-2 col-form-label">Local AP subnet</label>
            <div class="col-sm-10">
                <input type="text" class="form-control" id="local_ap_subnet">
            </div>
        </div>
        
        <div class="row mt-2">
            <label for="local_ap_gateway" class="col-sm-2 col-form-label">Local AP gateway</label>
            <div class="col-sm-10">
                <input type="text" class="form-control" id="local_ap_gateway">
            </div>
        </div>


        <div class="row mt-4">
            <h5>MQTT</h5>
        </div>
        <div class="row mt-2">
            <label for="mqtt_broker" class="col-sm-2 col-form-label">Mqtt broker / port</label>
            <div class="col-sm-8">
                <input type="text" class="form-control" id="mqtt_broker">
            </div>
            <div class="col-sm-2">
                <input type="text" class="form-control" id="mqtt_port">
            </div>
        </div>
        <div class="row mt-2">
            <label for="local_ap_gateway" class="col-sm-2 col-form-label">Mqtt topic</label>
            <div class="col-sm-10">
                <input type="text" class="form-control" id="mqtt_topic">
            </div>
        </div>
        <div class="row mt-2">
            <label for="local_ap_gateway" class="col-sm-2 col-form-label">Mqtt username</label>
            <div class="col-sm-10">
                <input type="text" class="form-control" id="mqtt_user">
            </div>
        </div>
        <div class="row mt-2">
            <label for="local_ap_gateway" class="col-sm-2 col-form-label">Mqtt password</label>
            <div class="col-sm-10">
                <input type="text" class="form-control" id="mqtt_password">
            </div>
        </div>

        <div class="row mt-4">
            <div class="col">
                <div class="form-check form-switch float-end">
                    <button id="save_settings" type="button" class="btn btn-primary" disabled>Save settings</button>
                </div>
            </div>
        </div>
    </div>

    <script src="bootstrap.min.js" type="text/javascript"></script>
    <script type="text/javascript">

        //Only used during tests, the real implementation uses relative urls
        const base_url = "";
         
        e_grill_name                = document.getElementById("grill_name");
        
        e_temperature_unit          = document.getElementById("temperature_unit");
        
        e_beep_enabled              = document.getElementById("beep_enabled");
        e_beep_volume               = document.getElementById("beep_volume");
        e_beep_degrees_before       = document.getElementById("beep_degrees_before");
        e_beep_outside_target       = document.getElementById("beep_outside_target");
        e_beep_on_ready             = document.getElementById("beep_on_ready");
        e_screen_timeout_minutes    = document.getElementById("screen_timeout_minutes");
        e_backlight_timeout_minutes = document.getElementById("backlight_timeout_minutes");
        e_backlight_brightness      = document.getElementById("backlight_brightness");
        
        e_wifi_scan_results         = document.getElementById("wifi_scan_results");
        
        e_wifi_ssid                 = document.getElementById("wifi_ssid");
        e_wifi_password             = document.getElementById("wifi_password");
        e_wifi_ip                   = document.getElementById("wifi_ip");
        e_wifi_subnet               = document.getElementById("wifi_subnet");
        e_wifi_gateway              = document.getElementById("wifi_gateway");
        e_wifi_dns                  = document.getElementById("wifi_dns");
        
        e_local_ap_ssid             = document.getElementById("local_ap_ssid");
        e_local_ap_password         = document.getElementById("local_ap_password");
        e_local_ap_ip               = document.getElementById("local_ap_ip");
        e_local_ap_subnet           = document.getElementById("local_ap_subnet");
        e_local_ap_gateway          = document.getElementById("local_ap_gateway");
        
        
        e_mqtt_broker               = document.getElementById("mqtt_broker");
        e_mqtt_port                 = document.getElementById("mqtt_port");
        e_mqtt_topic                = document.getElementById("mqtt_topic");
        e_mqtt_user                 = document.getElementById("mqtt_user");
        e_mqtt_password             = document.getElementById("mqtt_password");
        
        e_save_settings             = document.getElementById("save_settings");
        e_wifi_scan                 = document.getElementById("wifi_scan");

        e_alert                     = document.getElementById("alert");
        e_alert_text                = document.getElementById("alert-text");

        async function getSettings() {
            try {
                const response = await fetch(base_url + "/api/settings");
                data = await response.json();

                e_grill_name.value                = data['name'];

                e_temperature_unit.value          = data['temperature_unit'];
                
                e_beep_enabled.checked            = data['beep_enabled'];
                e_beep_volume.value               = data['beep_volume'];
                e_beep_degrees_before.value       = data['beep_degrees_before'];
                e_beep_outside_target.checked     = data['beep_outside_target'];
                e_beep_on_ready.checked           = data['beep_on_ready'];
                
                e_screen_timeout_minutes.value    = data['screen_timeout_minutes'];
                e_backlight_timeout_minutes.value = data['backlight_timeout_minutes'];
                e_backlight_brightness.value      = data['backlight_brightness'];

                e_wifi_ssid.value                 = data['wifi_ssid'];
                e_wifi_password.value             = data['wifi_password'];
                e_wifi_ip.value                   = data['wifi_ip'];
                e_wifi_subnet.value               = data['wifi_subnet'];
                e_wifi_gateway.value              = data['wifi_gateway'];
                e_wifi_dns.value                  = data['wifi_dns'];
                
                e_local_ap_ssid.value             = data['local_ap_ssid'];
                e_local_ap_password.value         = data['local_ap_password'];
                e_local_ap_ip.value               = data['local_ap_ip'];
                e_local_ap_subnet.value           = data['local_ap_subnet'];
                e_local_ap_gateway.value          = data['local_ap_gateway'];
                
                e_mqtt_broker.value               = data['mqtt_broker'];
                e_mqtt_port.value                 = data['mqtt_port'];
                e_mqtt_topic.value                = data['mqtt_topic'];
                e_mqtt_user.value                 = data['mqtt_user'];
                e_mqtt_password.value             = data['mqtt_password'];

                // Allow saving/scan once data is loaded
                e_save_settings.disabled = false;
                e_wifi_scan.disabled = false;


            } catch (error) {
                console.error('Grill is unreachable:', error);
            }
        }

        async function scanNetworks() {
            
            e_wifi_scan.textContent = "Scanning...";
            e_wifi_scan.disabled = true;
            e_wifi_scan_results.disabled = true;

            try {
                const response = await fetch(base_url + "/api/wifiscan");
                data = await response.json();

                // remove all options except for the first default element
                e_wifi_scan_results.options.length = 1;

                data.forEach((scanned_network) => {
                    var option = document.createElement('option');
                    option.value = scanned_network['ssid'];
                    option.innerHTML = scanned_network['ssid'] + " :: " + scanned_network['auth_method'] + " :: " + scanned_network['signal_strength'];
                    e_wifi_scan_results.appendChild(option);
                });
                
                e_wifi_scan.textContent = "Scan done!";
                e_wifi_scan.disabled = false;
                e_wifi_scan_results.disabled = false;


            } catch (error) {
                console.error('Grill is unreachable:', error);
            }
        }
        
        async function postSettings() {
            try {
                e_save_settings.disabled = true;
                
                e_probes = document.querySelectorAll('[data-probe]');

                post_data = {};
    
                post_data["name"]                      = e_grill_name.value;
                post_data["temperature_unit"]          = e_temperature_unit.value;
                post_data["beep_enabled"]              = e_beep_enabled.checked;
                post_data["beep_volume"]               = e_beep_volume.value;
                post_data["beep_degrees_before"]       = e_beep_degrees_before.value;
                post_data["beep_outside_target"]       = e_beep_outside_target.checked;
                post_data["beep_on_ready"]             = e_beep_on_ready.checked;
                post_data['screen_timeout_minutes']    = e_screen_timeout_minutes.value;
                post_data['backlight_timeout_minutes'] = e_backlight_timeout_minutes.value;
                post_data['backlight_brightness']      = e_backlight_brightness.value;

                post_data["wifi_ssid"]                 = e_wifi_ssid.value;
                post_data["wifi_password"]             = e_wifi_password.value;
                post_data["wifi_ip"]                   = e_wifi_ip.value;
                post_data["wifi_subnet"]               = e_wifi_subnet.value;
                post_data["wifi_gateway"]              = e_wifi_gateway.value;
                post_data["wifi_dns"]                  = e_wifi_dns.value;
                post_data["local_ap_ssid"]             = e_local_ap_ssid.value;
                post_data["local_ap_password"]         = e_local_ap_password.value;
                post_data["local_ap_ip"]               = e_local_ap_ip.value;
                post_data["local_ap_subnet"]           = e_local_ap_subnet.value;
                post_data["local_ap_gateway"]          = e_local_ap_gateway.value;
                post_data["mqtt_broker"]               = e_mqtt_broker.value;
                post_data["mqtt_port"]                 = e_mqtt_port.value;
                post_data["mqtt_topic"]                = e_mqtt_topic.value;
                post_data["mqtt_user"]                 = e_mqtt_user.value;
                post_data["mqtt_password"]             = e_mqtt_password.value;

                console.log(post_data)

                const response = await fetch(base_url + "/api/settings", {
                    method: "POST",
                    body: JSON.stringify(post_data),
                    headers: {
                        "Content-type": "application/json"
                    }
                });

                e_alert.classList.remove("alert-danger");
                e_alert.classList.add("alert-success");
                e_alert_text.textContent = "Settings have been saved successfully";
                e_alert.style.display = 'block';
                
                // Load settings
                getSettings();
                
            } catch (error) {
                e_alert.classList.add("alert-danger");
                e_alert.classList.remove("alert-success");
                e_alert_text.textContent = "Settings could not be saved";
                e_alert.style.display = 'block';

                console.error('Grill is unreachable:', error);
            }
        }

        getSettings();

        e_alert.style.display = 'none';

        e_save_settings.addEventListener('click', (event) => {
            postSettings();
        })

        e_wifi_scan.addEventListener('click', (event) => {
            scanNetworks();
        })

        e_wifi_scan_results.addEventListener('change', (event) => {
            result = e_wifi_scan_results.value
            
            if(result == "manual"){
                e_wifi_ssid.disabled = false;
            } else {
                e_wifi_ssid.disabled = true;
                e_wifi_ssid.value = result;
            }
        })

    </script>
  </body>
</html>
)=====";