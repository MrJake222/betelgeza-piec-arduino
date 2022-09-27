function circulation_update(json) {
    let time_left = json["circulation_pump_time_left"]
    let element = document.getElementById("circulation_pump")
    if (time_left === undefined) {
        element.innerHTML = "Wg. harmonogramu"
    }
    else {
        const zeroPad = (num, places) => String(num).padStart(places, '0')

        let min = Math.floor(time_left / 60)
        let sec = time_left % 60
        element.innerHTML = "Działa, " + zeroPad(min, 2) + ":" + zeroPad(sec, 2)
    }
}