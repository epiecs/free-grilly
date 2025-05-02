const char HTML_INDEX[] = R"=====(

<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Free Grilly</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-T3c6CoIi6uLrA9TneNEoa7RxnatzjcDSCmG1MXxSR1GAsXEV/Dwwykc2MPK8M2HN" crossorigin="anonymous">
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap-icons@1.11.3/font/bootstrap-icons.min.css">
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
                        <a class="nav-link active" aria-current="page" href="/">Temperatures</a>
                    </li>
                    <li class="nav-item">
                        <a class="nav-link" href="/probes">Probes</a>
                    </li>
                    <li class="nav-item">
                        <a class="nav-link" href="/settings">Settings</a>
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
        <div class="my-2 row">
            <div class="col-5 col-lg-7">
                <h5 id="grill-name">Free Grilly</h5>
            </div>
        
            <div class="col-7 col-lg-5 text-end">
                <span class="badge bg-success" id="grill-battery"><i id="grill-battery-charging" class="bi bi-lightning-charge-fill"></i> <span id="grill-battery-percentage">0</span>%</span>
                <span class="badge bg-danger" id="grill-wifi-connected"><i class="bi bi-wifi"></i> <span id="grill-wifi-strength">0%</span></span>
            </div>
        </div>
        <!-- <div class="row pt-1 row-cols-2 row-cols-sm-2 row-cols-md-2 row-cols-lg-4 g-2"> -->
        <div class="row">
            <div class="my-2 col-sm-12 col-md-6">
                <div class="card">
                    <div class="card-body py-1 px-2">
                        <div class="row">
                            <div class="col-8">Probe 1&nbsp;&nbsp;<span id="probe-1-status" class="badge text-dark bg-light"></span></div>
                            <div class="col-4 probe-temperature text-end" id="probe-1-temperature">0 C</div>
                            <div class="col-5"><small>Target temperature</small></div>
                            <div class="col-7 probe-temperature text-end"><small id="probe-1-target">0 C</small></div>
                            <div class="col-5"><small>Minimum temperature</small></div>
                            <div class="col-7 probe-temperature text-end"><small id="probe-1-minimum">0 C</small></div>
                        </div>
                    </div>
                </div>
            </div>
            <div class="my-2 col-sm-12 col-md-6">
                <div class="card">
                    <div class="card-body py-1 px-2">
                        <div class="row">
                            <div class="col-8">Probe 2&nbsp;&nbsp;<span id="probe-2-status" class="badge text-dark bg-light"></span></div>
                            <div class="col-4 probe-temperature text-end" id="probe-2-temperature">0 C</div>
                            <div class="col-5"><small>Target temperature</small></div>
                            <div class="col-7 probe-temperature text-end"><small id="probe-2-target">0 C</small></div>
                            <div class="col-5"><small>Minimum temperature</small></div>
                            <div class="col-7 probe-temperature text-end"><small id="probe-2-minimum">0 C</small></div>
                        </div>
                    </div>
                </div>
            </div>
            <div class="my-2 col-sm-12 col-md-6">
                <div class="card">
                    <div class="card-body py-1 px-2">
                        <div class="row">
                            <div class="col-8">Probe 3&nbsp;&nbsp;<span id="probe-3-status" class="badge text-dark bg-light"></span></div>
                            <div class="col-4 probe-temperature text-end" id="probe-3-temperature">0 C</div>
                            <div class="col-5"><small>Target temperature</small></div>
                            <div class="col-7 probe-temperature text-end"><small id="probe-3-target">0 C</small></div>
                            <div class="col-5"><small>Minimum temperature</small></div>
                            <div class="col-7 probe-temperature text-end"><small id="probe-3-minimum">0 C</small></div>
                        </div>
                    </div>
                </div>
            </div>
            <div class="my-2 col-sm-12 col-md-6">
                <div class="card">
                    <div class="card-body py-1 px-2">
                        <div class="row">
                            <div class="col-8">Probe 4&nbsp;&nbsp;<span id="probe-4-status" class="badge text-dark bg-light"></span></div>
                            <div class="col-4 probe-temperature text-end" id="probe-4-temperature">0 C</div>
                            <div class="col-5"><small>Target temperature</small></div>
                            <div class="col-7 probe-temperature text-end"><small id="probe-4-target">0 C</small></div>
                            <div class="col-5"><small>Minimum temperature</small></div>
                            <div class="col-7 probe-temperature text-end"><small id="probe-4-minimum">0 C</small></div>
                        </div>
                    </div>
                </div>
            </div>
            <div class="my-2 col-sm-12 col-md-6">
                <div class="card">
                    <div class="card-body py-1 px-2">
                        <div class="row">
                            <div class="col-8">Probe 5&nbsp;&nbsp;<span id="probe-5-status" class="badge text-dark bg-light"></span></div>
                            <div class="col-4 probe-temperature text-end" id="probe-5-temperature">0 C</div>
                            <div class="col-5"><small>Target temperature</small></div>
                            <div class="col-7 probe-temperature text-end"><small id="probe-5-target">0 C</small></div>
                            <div class="col-5"><small>Minimum temperature</small></div>
                            <div class="col-7 probe-temperature text-end"><small id="probe-5-minimum">0 C</small></div>
                        </div>
                    </div>
                </div>
            </div>
            <div class="my-2 col-sm-12 col-md-6">
                <div class="card">
                    <div class="card-body py-1 px-2">
                        <div class="row">
                            <div class="col-8">Probe 6&nbsp;&nbsp;<span id="probe-6-status" class="badge text-dark bg-light"></span></div>
                            <div class="col-4 probe-temperature text-end" id="probe-6-temperature">0 C</div>
                            <div class="col-5"><small>Target temperature</small></div>
                            <div class="col-7 probe-temperature text-end"><small id="probe-6-target">0 C</small></div>
                            <div class="col-5"><small>Minimum temperature</small></div>
                            <div class="col-7 probe-temperature text-end"><small id="probe-6-minimum">0 C</small></div>
                        </div>
                    </div>
                </div>
            </div>
            <div class="my-2 col-sm-12 col-md-6">
                <div class="card">
                    <div class="card-body py-1 px-2">
                        <div class="row">
                            <div class="col-8">Probe 7&nbsp;&nbsp;<span id="probe-7-status" class="badge text-dark bg-light"></span></div>
                            <div class="col-4 probe-temperature text-end" id="probe-7-temperature">0 C</div>
                            <div class="col-5"><small>Target temperature</small></div>
                            <div class="col-7 probe-temperature text-end"><small id="probe-7-target">0 C</small></div>
                            <div class="col-5"><small>Minimum temperature</small></div>
                            <div class="col-7 probe-temperature text-end"><small id="probe-7-minimum">0 C</small></div>
                        </div>
                    </div>
                </div>
            </div>
            <div class="my-2 col-sm-12 col-md-6">
                <div class="card">
                    <div class="card-body py-1 px-2">
                        <div class="row">
                            <div class="col-8">Probe 8&nbsp;&nbsp;<span id="probe-8-status" class="badge text-dark bg-light"></span></div>
                            <div class="col-4 probe-temperature text-end" id="probe-8-temperature">0 C</div>
                            <div class="col-5"><small>Target temperature</small></div>
                            <div class="col-7 probe-temperature text-end"><small id="probe-8-target">0 C</small></div>
                            <div class="col-5"><small>Minimum temperature</small></div>
                            <div class="col-7 probe-temperature text-end"><small id="probe-8-minimum">0 C</small></div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js" integrity="sha384-C6RzsynM9kWDrMNeT87bh95OGNyZPhcTNXj1NW7RuBCsyN/o0jlpcV8Qyq46cDfL" crossorigin="anonymous"></script>
    <script type="text/javascript">

        //Only used during tests, the real implementation uses relative urls
        const base_url = "";
        
        const api_polling_interval = 1000;
        const api_unreachable_intervals = 10;

        const sleep = ms => new Promise(res => setTimeout(res, ms));

        // Vars
        var temp_unit = "C";
        var status_classlist = ["text-dark", "bg-light", "bg-success", "bg-warning", "bg-danger"];


        // Selectors
        const e_probes = new Array(8);
        for(let probe_nr = 1; probe_nr < 9; probe_nr++){
            e_probes[probe_nr] = {};

            e_probes[probe_nr]["status"]              = document.getElementById("probe-" + probe_nr + "-status");
            e_probes[probe_nr]["temperature"]         = document.getElementById("probe-" + probe_nr + "-temperature");
            e_probes[probe_nr]["target_temperature"]  = document.getElementById("probe-" + probe_nr + "-target");
            e_probes[probe_nr]["minimum_temperature"] = document.getElementById("probe-" + probe_nr + "-minimum");
        }

        e_grill_name                      = document.getElementById("grill-name");

        e_grill_battery                   = document.getElementById("grill-battery");
        e_grill_battery_percentage        = document.getElementById("grill-battery-percentage");
        e_grill_battery_charging          = document.getElementById("grill-battery-charging");
        
        e_wifi_connected                  = document.getElementById("grill-wifi-connected");
        e_wifi_strength                   = document.getElementById("grill-wifi-strength");

        let wifi_signal_strength = 0;

        async function pollTemperatures() {
            
            try {
                const response = await fetch(base_url + "/api/grill");
                data = await response.json();
                
                // Grill name
                e_grill_name.textContent = data.name;

                switch(data.temperature_unit){
                    case "celcius":
                        temp_unit = "C";
                        break;
                    case "fahrenheit":
                        temp_unit = "F";
                        break;
                    default:
                        temp_unit = "?";
                }

                // Settings
                
                // Wifi
                if(data['wifi_connected'] == true){
                    e_wifi_connected.classList.remove("bg-danger");

                    wifi_signal_strength = 140 + data['wifi_signal'];
                    e_wifi_strength.textContent = wifi_signal_strength + " %";

                    if(wifi_signal_strength > 60){
                        e_wifi_connected.classList.add("bg-success");
                    } else {
                        e_wifi_connected.classList.add("bg-warning");
                    }
                } else {
                    e_wifi_connected.classList.remove("bg-success");
                    e_wifi_connected.classList.add("bg-danger");
                }

                // battery
                e_grill_battery_percentage.textContent = data.battery_percentage;

                if(data['battery_charging'] == true){
                    e_grill_battery_charging.style.display = "inline-block";
                } else {
                    e_grill_battery_charging.style.display = "none";
                }

                if(data.battery_percentage > 50){
                    e_grill_battery.classList.remove("bg-danger");
                    e_grill_battery.classList.remove("bg-warning");
                    e_grill_battery.classList.add("bg-success");
                } else if(data.battery_percentage > 30){
                    e_grill_battery.classList.remove("bg-danger");
                    e_grill_battery.classList.remove("bg-success");
                    e_grill_battery.classList.add("bg-warning");
                } else {
                    e_grill_battery.classList.remove("bg-warning");
                    e_grill_battery.classList.remove("bg-success");
                    e_grill_battery.classList.add("bg-danger");
                }

                for(let probe_nr = 1; probe_nr < 9; probe_nr++)
                {
                    if(data['probes'][probe_nr - 1]['connected'] == true){
                        e_probes[probe_nr]["temperature"].textContent = data['probes'][probe_nr - 1]['temperature'].toFixed(2) + " " + temp_unit;

                        //* target temperature mode
                        if(data['probes'][probe_nr - 1]['minimum_temperature'] == 0){

                            let temperature_difference = data['probes'][probe_nr - 1]['target_temperature'] - data['probes'][probe_nr - 1]['temperature']

                            if(temperature_difference >= 10){
                                e_probes[probe_nr]["status"].textContent = "Getting ready"
                                e_probes[probe_nr]["status"].classList.remove(...status_classlist);
                                e_probes[probe_nr]["status"].classList.add(...["bg-danger"]);
                            } else if(temperature_difference < 10 && temperature_difference > 0){
                                e_probes[probe_nr]["status"].textContent = "Almost ready"
                                e_probes[probe_nr]["status"].classList.remove(...status_classlist);
                                e_probes[probe_nr]["status"].classList.add(...["bg-warning"]);
                            } else if(temperature_difference <= 0 ){
                                e_probes[probe_nr]["status"].textContent = "Ready!"
                                e_probes[probe_nr]["status"].classList.remove(...status_classlist);
                                e_probes[probe_nr]["status"].classList.add(...["bg-success"]);
                            }
                        }

                        //* temperature range mode
                        if(data['probes'][probe_nr - 1]['minimum_temperature'] != 0){
                        
                            let temperature = data['probes'][probe_nr - 1]['temperature'];
                            let min_temp    = data['probes'][probe_nr - 1]['minimum_temperature'];
                            let max_temp    = data['probes'][probe_nr - 1]['target_temperature'];

                            if(temperature >= min_temp && temperature <= max_temp){
                                e_probes[probe_nr]["status"].textContent = "In range"
                                e_probes[probe_nr]["status"].classList.remove(...status_classlist);
                                e_probes[probe_nr]["status"].classList.add(...["bg-success"]);
                            } else {

                                let temperature_difference = 0;

                                if(temperature < min_temp){
                                    temperature_difference = min_temp - temperature;
                                }
                                if(temperature > max_temp){
                                    temperature_difference = temperature - max_temp;
                                }

                                if(temperature_difference >= 10){
                                    e_probes[probe_nr]["status"].textContent = "Way out of range"
                                    e_probes[probe_nr]["status"].classList.remove(...status_classlist);
                                    e_probes[probe_nr]["status"].classList.add(...["bg-danger"]);
                                } else if(temperature_difference < 10 && temperature_difference > 0){
                                    e_probes[probe_nr]["status"].textContent = "Out of range"
                                    e_probes[probe_nr]["status"].classList.remove(...status_classlist);
                                    e_probes[probe_nr]["status"].classList.add(...["bg-warning"]);
                                }
                            
                            }
                            
                        
                        }


                    }else{
                        e_probes[probe_nr]["temperature"].textContent = "-";

                        e_probes[probe_nr]["status"].textContent = "Disconnected"
                        e_probes[probe_nr]["status"].classList.remove(...status_classlist);
                        e_probes[probe_nr]["status"].classList.add(...["bg-light", "text-dark"]);
                    }
                    
                    e_probes[probe_nr]["minimum_temperature"].textContent = data['probes'][probe_nr - 1]['minimum_temperature'].toFixed(2) + " " + temp_unit;
                    e_probes[probe_nr]["target_temperature"].textContent  = data['probes'][probe_nr - 1]['target_temperature'].toFixed(2) + " " + temp_unit;
                }

            } catch (error) {
                console.error('Grill is unreachable:', error);
            }

        }
        
        async function pollingLoop() {
            await sleep(api_polling_interval);
            pollTemperatures();
            
            pollingLoop();
        }

        pollingLoop();

    </script>
  </body>
</html>


)=====";