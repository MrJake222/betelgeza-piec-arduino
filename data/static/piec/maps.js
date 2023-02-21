let pump_map = {
    0: "Ogrzewanie domu",
    1: "Priorytet bojlera",
    2: "Pompy równoległe",
    3: "Tryb letni",
}

let cu_map = {
    0x0000: "wygaszony",
    0x0001: "czuwanie",     // artificial
    0x0002: "praca",
    0x0003: "rozpalanie",   // artificial
    0x0004: "wygaszanie",   // artificial, not yet available

    // don't seem to be available
    0x0082: "nadzór",
    0x0009: "alarm: Temperatura nie rośnie",
    0x000E: "alarm: Czujnik CO uszkodzony",
    0x000A: "alarm: Czujnik podajnika uszkodzony",
    0x0031: "rozpalanie",
    0x0027: "alarm: Czujnik podłogi uszkodzony",
    0x0029: "alarm: Czujnik zaworu uszkodzony",
    0x0052: "tryb nadzoru",
}

function maps_update(json) {
    document.getElementById("pumps_mode").innerHTML = pump_map[json["pumps_mode"]]
    document.getElementById("control_unit_mode").innerHTML = cu_map[json["cu_state"]]
}

Object.keys(pump_map).forEach(key => {
    let opt = document.createElement("option")
    opt.value = key
    opt.innerHTML = pump_map[key]
    document.getElementById("pumps_modes").appendChild(opt)
})