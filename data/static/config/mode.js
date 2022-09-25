function set_up_status(loading, radioOn, radioOff, isOn) {
    loading.style.animation = "none"
    loading.style.color = isOn ? "green" : "red"
    loading.innerHTML = isOn ? "ON" : "OFF"
    radioOn.checked = isOn
    radioOff.checked = !isOn
}

function set_up_error(loading, text) {
    loading.style.animation = "none"
    loading.style.color = "red"
    loading.innerHTML = text
}

async function load_mode() {
    console.info("loading mode")
    
    let loadings = {}
    loadings["ap"] = document.getElementById("loading_mode_ap")
    loadings["sta"] = document.getElementById("loading_mode_sta")
    let radiosOn = {}
    radiosOn["ap"] = document.getElementById("radio_ap_on")
    radiosOn["sta"] = document.getElementById("radio_sta_on")
    let radiosOff = {}
    radiosOff["ap"] = document.getElementById("radio_ap_off")
    radiosOff["sta"] = document.getElementById("radio_sta_off")

    try {
        let resp = await fetch("/wifi_mode")
        console.debug("response mode: ", resp)

        if (!resp.ok) {
            throw Error("response mode not ok, error: " + resp.statusText)
        }

        let json = await resp.json()
        console.debug("Mode: ", json)

        let mode = json["mode"]
        if (!["off", "ap", "sta", "ap_sta"].includes(mode)) {
            throw Error("unknown mode, error: " + resp.statusText)
        }

        let ap = (mode == "ap" || mode == "ap_sta")
        let sta = (mode == "sta" || mode == "ap_sta")

        document.getElementById("radio_ap_on").checked = ap
        document.getElementById("radio_ap_off").checked = !ap

        document.getElementById("radio_sta_on").checked = sta
        document.getElementById("radio_sta_off").checked = !sta

        set_up_status(loadings["ap"], radiosOn["ap"], radiosOff["ap"], ap)
        set_up_status(loadings["sta"], radiosOn["sta"], radiosOff["sta"], sta)

        if (ap) {
            loadings["ap"].innerHTML += "<br>Name: " + json["ap_ssid"]
            loadings["ap"].innerHTML += "<br>IP: " + json["ap_ip"]
        }

        loadings["sta"].innerHTML += "<br>" + json["sta_status_string"]
        if (json["sta_status_string"].includes("Connected")) {
            loadings["sta"].innerHTML += "<br>IP: " + json["sta_ip"]
        }
        else {
            loadings["sta"].style.color = "red"
        }
    }
    catch (e) {
        console.error("failed to load mode")
        console.error("error: " + e)
        set_up_error(loadings["ap"], "Failed, see console for details")
        set_up_error(loadings["sta"], "Failed, see console for details")
    }
}