const id_list = [
    "co_current",
    "co_target",
    "cwu_current",
    "cwu_target",
    "valve_current",
    "valve_target"
]

let first_temp = true

function temp_update(json) {
    id_list.forEach(id => {

        let val = json[id]
        let element = document.getElementById(id)

        if (val === undefined) {
            element.innerHTML = "--"
        } else {
            element.innerHTML = val
        }
    })

    if (first_temp) {
        first_temp = false

        const co_set = document.getElementById("co_set");
        co_set.value = json["co_target"]
        co_set.min = json["co_min"]
        co_set.max = json["co_max"]

        const cwu_set = document.getElementById("cwu_set");
        cwu_set.value = json["cwu_target"]
        cwu_set.min = json["cwu_min"]
        cwu_set.max = json["cwu_max"]

        // const valve_set = document.getElementById("valve_set");
        // valve_set.value = json["valve_target"] !== undefined ? json["valve_target"] : 30
        // valve_set.min = json["valve_min"] !== undefined ? json["valve_min"] : 0
        // valve_set.max = json["valve_max"] !== undefined ? json["valve_max"] : 100

        document.getElementById("pumps_modes").value = json["pumps_mode"]
    }
}